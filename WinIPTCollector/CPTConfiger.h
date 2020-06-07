#pragma once
#include "..//share/itf/itf.h"
#include "CToPAConstructor.h"

class CPTConfiger
{
public:
	explicit CPTConfiger(int cpuIdx) : cpuIdx_(cpuIdx) {}
	virtual ~CPTConfiger();
	bool IsPtRunning();
	void StopPt();
	void SetAddrN(int idx, const AddrRange &addr)
	{
		addr_[idx].addrN_A = addr.addrN_A;
		addr_[idx].addrN_B = addr.addrN_B;
		addr_[idx].cfgMode = addr.cfgMode;
		addrN_ = idx + 1;
	}
	void SetParam(unsigned int buffSize, unsigned int pid, unsigned long long cr3, unsigned int mtcFreq, unsigned int psbFreq, unsigned int cycThld) {
		buffSize_ = buffSize;
		pid_ = pid;
		cr3_ = cr3;
		mtcFreq_ = mtcFreq;
		psbFreq_ = psbFreq;
		cycThld_ = cycThld;
	}
	int SetupPt();
	void* GetToPAHeader() {
		return topa_[curToPAIdx_].GetToPAHeader();
	}
	unsigned int GetBuffSize()
	{
		return buffSize_;
	}
	void* GetOutputBuff()
	{
		return topa_[curToPAIdx_].GetOutputBuff();
	}
	int GetIdx() {
		return cpuIdx_;
	}
	int GetTableIdx(unsigned long long address) {
		return topa_[curToPAIdx_].GetTableIdx(address);
	}
	int SwapToPA();
	void *GetBuffByIdx(int idx) {
		return topa_[idx].GetOutputBuff();
	}
	int GetBuffIdx() {
		return curToPAIdx_;
	}
private:
	int SetupToPA(unsigned int buffSize);
	int SetupPtCtl();
	unsigned long long  GetProcessCr3(unsigned int pid);
	CToPAConstructor topa_[2];
	int curToPAIdx_ = 0;
	int cpuIdx_ = -1;
	unsigned int buffSize_ = 0;
	unsigned int mtcFreq_ = 0;
	unsigned int cycThld_ = 0;
	unsigned int pid_ = 0;
	unsigned long long cr3_ = 0;
	unsigned int psbFreq_ = 0;
	AddrRange addr_[4];
	int addrN_ = 0;
};


