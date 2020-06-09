#include "ConfigMain.h"
#include "CPTConfiger.h"
#include "InnerDataStru.h"
#include "CNotifierMng.h"
#include "ARCH.h"
CPTConfiger* g_allConfiger[32] = { 0 };

void SetupPTFunc(void* param)
{
	PtSetupInfoInner* setupInfo = (PtSetupInfoInner*)param;
	auto curCpu = ARCH::GetCurrentCpuIdx();
	auto configer = new CPTConfiger(curCpu);
	configer->SetParam(setupInfo->setupInfo.buffSize * 1024 * 1024, setupInfo->setupInfo.pid, setupInfo->cr3,
		setupInfo->setupInfo.mtcFreq, setupInfo->setupInfo.psbFreq, setupInfo->setupInfo.cycThld);
	for (auto i = 0; i < 4; ++i) {
		if (setupInfo->setupInfo.addrsCfg[i].cfgMode != ARCM_UNUSED) {
			configer->SetAddrN(i, setupInfo->setupInfo.addrsCfg[i]);
		}
	}
	setupInfo->setupRst = configer->SetupPt();
	g_allConfiger[curCpu] = configer;
}
void StopPTFun(void* param)
{
	param = param;
	auto curCpu = ARCH::GetCurrentCpuIdx();
	g_allConfiger[curCpu]->StopPt();
}

void SetupNotifier(PtSetupInfo* info, PtSetupRst *rst)
{
	// Must setup configer before, because configer is running in DPC level
	unsigned int num = 0;
	for (unsigned int i = 0; i < info->cpuNum; ++i) {
		auto configer = g_allConfiger[i];
		if (configer == nullptr) {
			continue;
		}
		auto notifier = new CNotifier();
		if (notifier == nullptr) {
			continue;
		}
		notifier->SetParam(configer->GetIdx(), configer);
		AddNotifier(notifier);
		num++;
	}
	GetOutputAddress(rst->outBufferInfo, rst->outBuffNum, rst->outBufferLen);
}
void SetupPtMain(void* info, void* rstInfo)
{
	auto para = (PtSetupInfo*)info;

	PtSetupInfoInner InnerInfo;
	InnerInfo.setupInfo = *para;
	InnerInfo.setupRst = 1;
	InnerInfo.cr3 = ARCH::GetProcessCr3(para->pid);
	ARCH::ForEachCpu(SetupPTFunc, &InnerInfo);
	SetupNotifier(para, (PtSetupRst *)rstInfo);
	((PtSetupRst*)rstInfo)->rst = InnerInfo.setupRst;
}

void StopPtMain()
{
	ARCH::ForEachCpu(StopPTFun, nullptr);
	ClearAllNotifier();
	for (auto i = 0; i < 32; ++i) {
		if (g_allConfiger[i] != nullptr) {
			delete g_allConfiger[i];  // free memory
			g_allConfiger[i] = nullptr;
		}
	}
	}
	
