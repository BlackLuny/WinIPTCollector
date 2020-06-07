#include "CPtkGenCfg.h"
#include <fstream>
#include "ToolFunc.h" 
#include "ConfigINI.h" 
#include "ProceessUtils.h"
CPtkGenCfg* CPtkGenCfg::instance_ = nullptr;
bool IsSupportPt()
{
	int rst[4];
	__cpuidex(rst, 0x7, 0);
	unsigned int ebx = rst[1];
	return (ebx >> 25) & 1;
}
void GetSupportMtcFreqLst(std::vector<int>& lst)
{
	int rst[4];
	struct PtCpuId0x14_1InfoEax {
		union {
			struct {
				UINT32 AddressRangeNum : 3;
				UINT32 rsvd : 13;
				UINT32 MTCPeriodBitmap : 16;
			}Bits;
			UINT32 value;
		};
	}capForOneEax_ = { 0 };
	__cpuidex(rst, 0x14, 1);
	capForOneEax_.value = rst[0];
	auto bitMap = capForOneEax_.Bits.MTCPeriodBitmap;
	for (int i = 0; i < 16; ++i) {
		if (bitMap & 1) {
			lst.push_back(i);
		}
		bitMap >>= 1;
	}

}

int GetSupportAddrN()
{
	int rst[4];
	struct PtCpuId0x14_0InfoEbx {
		union {
			struct {
				UINT32 cr3FilterCap : 1;
				UINT32 psbCycCap : 1;
				UINT32 ipFilter : 1;
			}Bits;
			UINT32 value;
		};
	}capFor0Ebx_ = { 0 };
	__cpuidex(rst, 0x14, 0);
	capFor0Ebx_.value = rst[1];
	if (!capFor0Ebx_.Bits.ipFilter) {
		return 0;
	}
	struct PtCpuId0x14_1InfoEax {
		union {
			struct {
				UINT32 AddressRangeNum : 3;
				UINT32 rsvd : 13;
				UINT32 MTCPeriodBitmap : 16;
			}Bits;
			UINT32 value;
		};
	}capForOneEax_ = { 0 };
	__cpuidex(rst, 0x14, 1);
	capForOneEax_.value = rst[0];
	return capForOneEax_.Bits.AddressRangeNum;
}

void GetSupportPSBFreqLst(std::vector<int>& lst)
{
	int rst[4];
	struct PtCpuId0x14_1InfoEbx {
		union {
			struct {
				UINT32 CycleThdBitmap : 16;
				UINT32 PSBFreqBitmap : 16;
			}Bits;
			UINT32 value;
		};
	}capForOneEbx_ = { 0 };
	__cpuidex(rst, 0x14, 1);
	capForOneEbx_.value = rst[1];
	auto bitMap = capForOneEbx_.Bits.PSBFreqBitmap;
	for (int i = 0; i < 16; ++i) {
		if (bitMap & 1) {
			lst.push_back(i);
		}
		bitMap >>= 1;
	}

}

void GetSupportCycThldLst(std::vector<int>& lst)
{
	int rst[4];
	struct PtCpuId0x14_1InfoEbx {
		union {
			struct {
				UINT32 CycleThdBitmap : 16;
				UINT32 PSBFreqBitmap : 16;
			}Bits;
			UINT32 value;
		};
	}capForOneEbx_ = { 0 };
	__cpuidex(rst, 0x14, 1);
	capForOneEbx_.value = rst[1];
	auto bitMap = capForOneEbx_.Bits.CycleThdBitmap;
	for (int i = 0; i < 16; ++i) {
		if (bitMap & 1) {
			if (i == 0) {
				lst.push_back(0);
			}
			else {
				lst.push_back(1 << (i - 1));
			}
		}
		bitMap >>= 1;
	}

}

bool CheckCfg()
{
	auto ins = CPtkGenCfg::Instance();
	if (ins->getBufferSize() == 0) {
		return false;
	}
	if (!IsSupportPt()) {
		std::cout << "Your processor is not support PT.\n";
		return false;
	}
	std::vector<int> mtcFreqSupportLst;
	GetSupportMtcFreqLst(mtcFreqSupportLst);
	auto itr = std::find(mtcFreqSupportLst.begin(), mtcFreqSupportLst.end(), ins->getMtcFreq());
	if (itr == mtcFreqSupportLst.end()) {
		std::cout << "Not support mtc freq:" << ins->getMtcFreq() << std::endl;
		return false;
	}
	std::vector<int> psbFreqSupportLst;
	GetSupportPSBFreqLst(psbFreqSupportLst);
	itr = std::find(psbFreqSupportLst.begin(), psbFreqSupportLst.end(), ins->getPsbFreq());
	if (itr == psbFreqSupportLst.end()) {
		std::cout << "Not support psb freq:" << ins->getPsbFreq() << std::endl;
		return false;
	}
	std::vector<int> cycThldSupportLst;
	GetSupportCycThldLst(cycThldSupportLst);
	itr = std::find(cycThldSupportLst.begin(), cycThldSupportLst.end(), ins->getCycThld());
	if (itr == cycThldSupportLst.end()) {
		std::cout << "Not support cyc thld:" << ins->getCycThld() << std::endl;
		return false;
	}
	return true;
}


CPtkGenCfg* CPtkGenCfg::Instance()
{

	if (instance_ == nullptr) {
		instance_ = new CPtkGenCfg();
	}
	return instance_;
}


bool CPtkGenCfg::Init(const char* iniFile)
{
	auto file = std::ifstream(iniFile, std::ios::in);
	INI::Parser p(file);
	auto ins = Instance();
	ins->bufferSize_ = strtoul(p.top()("PTCONFIG")["BufferSize"].c_str(), nullptr, 10);
	ins->psbFreq_ = strtoul(p.top()("PTCONFIG")["PsbFreq"].c_str(), nullptr, 10);
	ins->mtcFreq_ = strtoul(p.top()("PTCONFIG")["MTCFreq"].c_str(), nullptr, 10);
	ins->cycThld_ = strtoul(p.top()("PTCONFIG")["CycThld"].c_str(), nullptr, 10);
	ins->printTimeFlag_ = strtoul(p.top()("PTCONFIG")["PrintTime"].c_str(), nullptr, 10) != 0;
	auto addrNum = GetSupportAddrN();
	std::vector<ModuleInfo> moduleInfo;
	GetProcessModulesInfo(ins->getPid(), moduleInfo);
	for (auto i = 0; i < addrNum; ++i) {
		char tmp[16] = { 0 };
		sprintf_s(tmp, "ADDR%d_CFG", i);
		auto v = p.top()("PTCONFIG")[tmp];
		ins->addrFilter_[i].cfg = strtoul(v.c_str(), nullptr, 10);
		sprintf_s(tmp, "ADDR%d_A", i);
		v = p.top()("PTCONFIG")[tmp];
		if (v.find(".") == -1) {
			ins->addrFilter_[i].addr_A = strtoull(v.c_str(), nullptr, 16);
		}
		else {
			auto module = QueryModuleInfo(v, moduleInfo);
			ins->addrFilter_[i].addr_A = module.base;
			std::cout << "addr" << i << "_A = " << (void*)ins->addrFilter_[i].addr_A << std::endl;
		}
		sprintf_s(tmp, "ADDR%d_B", i);
		v = p.top()("PTCONFIG")[tmp];
		if (v.find(".") == -1) {
			ins->addrFilter_[i].addr_B = strtoull(v.c_str(), nullptr, 16);
		}
		else {
			auto module = QueryModuleInfo(v, moduleInfo);
			ins->addrFilter_[i].addr_B = module.base + module.size;
			std::cout << "addr" << i <<"_B = " << (void *)ins->addrFilter_[i].addr_B << std::endl;
		}
	}
	file.close();
	return CheckCfg();
}