#pragma once
class CPtkGenCfg
{
public:
	static CPtkGenCfg* Instance();
	static bool Init(const char* iniFile);
	unsigned int getPid() {
		return pid_;
	}
	void setPid(unsigned int pid) {
		pid_ = pid;
	}
	unsigned long long getNomFreq() {
		return nomFreq_;
	}
	void setNomFreq(unsigned long long nomFreq) {
		nomFreq_ = nomFreq;
	}
	unsigned int getBufferSize() {
		return bufferSize_;
	}
	unsigned int getMtcFreq() {
		return mtcFreq_;
	}
	unsigned int getCycThld() {
		return cycThld_;
	}
	unsigned int getPsbFreq() {
		return psbFreq_;
	}
	unsigned int getAddrCfg(int idx) {
		return addrFilter_[idx].cfg;
	}
	unsigned long long getAddrA(int idx) {
		return addrFilter_[idx].addr_A;
	}
	unsigned long long getAddrB(int idx) {
		return addrFilter_[idx].addr_B;
	}
	bool GetPrintTimeFlag()
	{
		return printTimeFlag_;
	}
private:
	CPtkGenCfg() = default;
	struct AddrFilterCfgInfo {
		int cfg;
		unsigned long long addr_A;
		unsigned long long addr_B;
	};
	unsigned long long nomFreq_ = 0;
	unsigned int pid_ = 0;
	unsigned int bufferSize_ = 256;
	unsigned int mtcFreq_ = 5;
	unsigned int psbFreq_ = 5;
	unsigned int cycThld_ = 5;
	AddrFilterCfgInfo addrFilter_[4] = {0};
	bool printTimeFlag_ = false;
	static CPtkGenCfg* instance_;
};

