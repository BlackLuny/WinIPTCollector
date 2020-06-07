#include "ToolFunc.h"
#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>
#include <tlhelp32.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "ProceessUtils.h"
using namespace std;
#include "..//itf/ioctl_code.h"
#include "..//itf/itf.h"
#define DEVICE_NAME L"\\\\.\\PtCollector"
#define MSR_PLATFORM_INFO 0xce
std::string FillTailSpace(const string &input, int expectSize)
{
	if (input.size() >= expectSize) {
		return input;
	}
	else {
		std::string rst = input;
		auto spaceNum = expectSize - input.size();
		while (spaceNum-- > 0) {
			rst += " ";
		}
		return rst;
	}
}
std::string RemoveHeadZeros(const string& input)
{
	std::string rst;
	auto idx = 0;
	while (input.at(idx) == '0') {
		idx++;
	}

	rst = input.substr(idx);
	return rst;
}

void ReadMemoryByDrv(unsigned int pid, unsigned long long address, unsigned int len,
	void* outputBuff, unsigned int outputBuffSize, size_t* outputSize)
{
	unsigned int readSize = 0;
	ReadProcessMemoryBB(pid, address, len, outputBuff, outputBuffSize, &readSize);
	*outputSize = readSize;
}

size_t ReadMemory(HANDLE process, char* buffer, size_t size,
	unsigned long long addr)
{
	SIZE_T readSize = 0;
	if (!ReadProcessMemory(process, (LPVOID)(addr), buffer, size, &readSize)) {
		return 0;
	}
	return readSize;
}
void Wchar_tToString(std::string& szDst, wchar_t* wchar)
{
	wchar_t* wText = wchar;
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte的运用
	char* psText; // psText为char*的临时数组，作为赋值给std::string的中间变量
	psText = new char[dwNum];
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte的再次运用
	szDst = psText;// std::string赋值
	delete[]psText;// psText的清除
}

// string to wstring
void StringToWstring(std::wstring& szDst, std::string str)
{
	std::string temp = str;
	int len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, NULL, 0);
	wchar_t* wszUtf8 = new wchar_t[len + 1];
	memset(wszUtf8, 0, len * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, (LPWSTR)wszUtf8, len);
	szDst = wszUtf8;
	std::wstring r = wszUtf8;
	delete[] wszUtf8;
}

std::string WStringToString(const std::wstring& wstr)
{
	std::string str;
	int nLen = (int)wstr.length();
	str.resize(nLen, ' ');
	int nResult = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wstr.c_str(), nLen, (LPSTR)str.c_str(), nLen, NULL, NULL);
	if (nResult == 0)
	{
		return "";
	}
	return str;
}

unsigned long long Hex2Ull(std::string&& str)
{

}
void GetRuntimePidInfo(vector <PidInfo>& list)
{
	HANDLE procSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (procSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot failed, %d ", GetLastError());
		return;
	}
	//
	PROCESSENTRY32 procEntry = { 0 };
	procEntry.dwSize = sizeof(PROCESSENTRY32);
	BOOL bRet = Process32First(procSnap, &procEntry);
	while (bRet)
	{
		//wprintf(L"PID: %d (%s) ", procEntry.th32ProcessID, procEntry.szExeFile);
		PidInfo info;
		info.pid = procEntry.th32ProcessID;
		Wchar_tToString(info.pidName, procEntry.szExeFile);
		list.push_back(info);
		bRet = Process32Next(procSnap, &procEntry);
	}
	CloseHandle(procSnap);
}


DWORD GetPid(string& pidName)
{
	vector <PidInfo> list;
	GetRuntimePidInfo(list);
	std::transform(pidName.begin(), pidName.end(), pidName.begin(), ::toupper);
	for (auto& cur : list) {
		std::transform(cur.pidName.begin(), cur.pidName.end(), cur.pidName.begin(), ::toupper);
		if (cur.pidName == pidName) {
			return cur.pid;
		}
	}
	return -1;
}
