#include <iostream>
#include <Windows.h>
#include "../share/itf/itf.h"
#include "../share/itf/ioctl_code.h"
#include "../share/common/ToolFunc.h"
#include "../share/common/CPtkGenCfg.h"
#include <algorithm>
#include "ThreadWraperNoPMI.h"
#define DEVICE_NAME L"\\\\.\\WinIPTCollecctor"
#define MSR_PLATFORM_INFO 0xce
HANDLE g_Device = INVALID_HANDLE_VALUE;

void CommunicateWithDevice(unsigned int code, void* inputInfo, unsigned int inputSize,
	void* outputInfo, unsigned int outputBuffSize, unsigned int* outputSize)
{
	if (g_Device == INVALID_HANDLE_VALUE) {
		g_Device = CreateFile(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	HANDLE hDevice = g_Device;
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		DWORD dwBytes = 0;
		BOOL b = DeviceIoControl(hDevice, code,
			inputInfo, inputSize, outputInfo, outputBuffSize, &dwBytes, NULL);
		if (outputSize != nullptr) {
			*outputSize = dwBytes;
		}
	}
	else
		printf("CreateFile failed, for code:%x err: %x\n", code, GetLastError());
}

#include <memory>
#include <vector>
std::vector<ThreadWraperNoPMI*> g_allThreadNoPMI;
void CreateThreadWraperNoPMI(DWORD cpuNum)
{
	for (DWORD i = 0; i < cpuNum; ++i) {
		ThreadWraperNoPMI* thdWraper = new ThreadWraperNoPMI;
		g_allThreadNoPMI.push_back(thdWraper);
	}
}
void CreateThread4NoPMI()
{
	DWORD threadId = 0;
	if (NULL == CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ThreadWraperNoPMI::Run, &g_allThreadNoPMI, false, &threadId)) {
		std::cout << "Fail while create thread\n" << std::endl;
	}
}

void SetupOutBufferNoPMI(PtSetupRst& setupRstInfo)
{
	for (auto i = 0; i < g_allThreadNoPMI.size(); ++i) {
		auto thread = g_allThreadNoPMI[i];
		thread->SetOutBuff((void*)setupRstInfo.outBufferInfo[i]);
		thread->SetBufferLen(setupRstInfo.outBufferLen);
		std::cout << "Buffer:" << (void*)setupRstInfo.outBufferInfo[i] << " len: " << setupRstInfo.outBufferLen << std::endl;
	}
	Sleep(1000);
}

int SetupPtNoPmi(DWORD pid, DWORD buffSize, DWORD mtcFreq, DWORD psbFeq, DWORD cycThld, unsigned int addrCfg, unsigned long long addrStart, unsigned long long addrEnd)
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	auto cpuNum = sysInfo.dwNumberOfProcessors;

	PtSetupInfo setupInfo = { 0 };
	setupInfo.pid = pid;
	setupInfo.buffSize = buffSize;
	setupInfo.retCompress = false;
	setupInfo.mtcFreq = mtcFreq;
	setupInfo.psbFreq = psbFeq;
	setupInfo.cycThld = cycThld;
	setupInfo.cpuNum = cpuNum;
	setupInfo.addrsCfg[0].cfgMode = (AddrRangeCfgMode)addrCfg;
	setupInfo.addrsCfg[0].addrN_A = addrStart;
	setupInfo.addrsCfg[0].addrN_B = addrEnd;

	// Create collectors
	CreateThreadWraperNoPMI(cpuNum);

	PtSetupRst rst = { 0 };
	unsigned int outputSize = 0;
	CommunicateWithDevice(IOCTL_SETUP_PT, &setupInfo, sizeof(setupInfo),
		&rst, sizeof(rst), &outputSize);
	if (outputSize != sizeof(rst)) {
		std::cout << "Setup PT outputSize error actuall size:" << outputSize
			<< " expected size:" << sizeof(rst) << std::endl;
		return -1;
	}
	if (rst.rst != 0) {
		std::cout << "Setup PT rst error:" << rst.rst << std::endl;
		return -1;
	}
	// After setup PT, setup output buffer
	SetupOutBufferNoPMI(rst);
	// Start collecting data
	CreateThread4NoPMI();
	return 0;
}
extern "C" void SetupServerPid(unsigned int serverPid)
{
	auto ourPID = serverPid;
	PtSetupServerPid info;
	info.pid = ourPID;
	PtSetupServerPidRsp rst = { 0 };
	unsigned int outputSize = 0;
	CommunicateWithDevice(IOCTL_SETUP_SERVER_PID, &info, sizeof(info),
		&rst, sizeof(rst), &outputSize);
	if (outputSize != sizeof(rst)) {
		std::cout << "Setup server pid outputSize error actuall size:" << outputSize
			<< " expected size:" << sizeof(rst) << std::endl;
		std::cout << "Error: " << GetLastError() << std::endl;
		return;
	}
}

unsigned long long ReadNomFreq()
{
	PtReadMsr info;
	info.msrId = MSR_PLATFORM_INFO;
	PtReadMsrRst rst = { 0 };
	unsigned int outputSize = 0;
	CommunicateWithDevice(IOCTL_READ_MSR, &info, sizeof(info),
		&rst, sizeof(rst), &outputSize);
	if (outputSize != sizeof(rst)) {
		std::cout << "Read msr error actuall size:" << outputSize
			<< " expected size:" << sizeof(rst) << std::endl;
		return 0;
	}
	std::cout << "NomFreq = " << rst.rst << std::endl;
	return rst.rst;
}

void SetupNoPmi()
{
	auto rst = SetupPtNoPmi(CPtkGenCfg::Instance()->getPid(),
		CPtkGenCfg::Instance()->getBufferSize(), CPtkGenCfg::Instance()->getMtcFreq(),
		CPtkGenCfg::Instance()->getPsbFreq(), CPtkGenCfg::Instance()->getCycThld(),
		CPtkGenCfg::Instance()->getAddrCfg(0), CPtkGenCfg::Instance()->getAddrA(0), CPtkGenCfg::Instance()->getAddrB(0));
}

extern "C" int CollectIPTByPID(unsigned int pid)
{
	CPtkGenCfg::Instance()->setPid(pid);
	std::string cfgFile;
	Wchar_tToString(cfgFile, (wchar_t*)L"config.ini");
	if (!CPtkGenCfg::Init(cfgFile.c_str())) {
		std::cout << "Environment check failed!\n";
		return 0;
	}
	CPtkGenCfg::Instance()->setNomFreq(ReadNomFreq());
	SetupNoPmi();
	while (1) {
	}
	CloseHandle(g_Device);
	return 0;
}

extern "C" int CollectIPTByProcessName(const char *procName)
{
	std::string processName = procName;
	auto pid = GetPid(processName);
	if (pid == -1) {
		std::cout << "Process " << processName << " is not running!\n";
		return 0;
	}
	CollectIPTByPID(pid);
}