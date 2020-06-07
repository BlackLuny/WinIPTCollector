#include "ARCH.h"
#include "CPTConfiger.h"
#include "CToPAConstructor.h"
#include "PTDef.h"


int CPTConfiger::SetupToPA(unsigned int buffSize)
{
	// 如果有之前的数据，先释放掉，因为可能这次的buffSize不一样了

	topa_[0].DeconstructToPA();
	buffSize_ = topa_[0].ConstructToPA(buffSize);
	curToPAIdx_ = 0;
	return 0;
}

int CPTConfiger::SetupPt()
{
	if (IsPtRunning()) {
		StopPt();
	}
	auto rst = SetupToPA(buffSize_);
	if (rst != 0) {
		return rst;
	}
	rst = SetupPtCtl();
	if (rst != 0) {
		return rst;
	}
	return rst;
}


int CPTConfiger::SetupPtCtl()
{
	auto cr3 = GetProcessCr3(pid_);
	RTIT_CTL ctl;
	RTIT_STATUS s;
	int i = 0;
	__try
	{
		ctl.Value = ARCH::ReadMsr(IA32_RTIT_CTL);
		i = 1;
	}
	__except (1)
	{
		DbgPrint("ultimap2_setup_dpc: IA32_RTIT_CTL in unreadable");
		return i;
	}

	ctl.Bits.TraceEn = 1;

	ctl.Bits.OS = 0;

	ctl.Bits.USER = 1;

	ctl.Bits.CR3Filter = 1;


	ctl.Bits.ToPA = 1;
	if (mtcFreq_ > 0) {
		ctl.Bits.TSCEn = 1;
		ctl.Bits.MTCEn = 1;
		ctl.Bits.MTCFreq = mtcFreq_;
	}
	if (cycThld_ > 0) {
		ctl.Bits.TSCEn = 1;
		ctl.Bits.CYCEn = 1;
		ctl.Bits.CycThresh = cycThld_;
	}
	ctl.Bits.PSBFreq = psbFreq_;
	ctl.Bits.DisRETC = 0;
	ctl.Bits.BranchEn = 1;
	__try
	{
		if (addrN_ >= 1) {
			ctl.Bits.ADDR0_CFG = addr_[0].cfgMode;
			ARCH::WriteMsr(IA32_RTIT_ADDR0_A, addr_[0].addrN_A);
			ARCH::WriteMsr(IA32_RTIT_ADDR0_B, addr_[0].addrN_B);
		}
		if (addrN_ >= 2) {
			ctl.Bits.ADDR1_CFG = addr_[1].cfgMode;
			ARCH::WriteMsr(IA32_RTIT_ADDR1_A, addr_[1].addrN_A);
			ARCH::WriteMsr(IA32_RTIT_ADDR1_B, addr_[1].addrN_B);
		}
		if (addrN_ >= 3) {
			ctl.Bits.ADDR2_CFG = addr_[2].cfgMode;
			ARCH::WriteMsr(IA32_RTIT_ADDR2_A, addr_[2].addrN_A);
			ARCH::WriteMsr(IA32_RTIT_ADDR2_B, addr_[2].addrN_B);
		}
		if (addrN_ >= 4) {
			ctl.Bits.ADDR3_CFG = addr_[3].cfgMode;
			ARCH::WriteMsr(IA32_RTIT_ADDR3_A, addr_[3].addrN_A);
			ARCH::WriteMsr(IA32_RTIT_ADDR3_B, addr_[3].addrN_B);
		}

	}
	__except (1)
	{
		DbgPrint("ctl.Value=%p\n", ctl.Value);
		DbgPrint("CR3=%p\n", cr3);
		DbgPrint("faile at i =%u\n", i);
		return i;
	}

	__try
	{

		ARCH::WriteMsr(IA32_RTIT_OUTPUT_BASE, ARCH::GetPhyAddress(topa_[curToPAIdx_].GetToPAHeader()).QuadPart);
		i = 2;
		ARCH::WriteMsr(IA32_RTIT_OUTPUT_MASK_PTRS, 0);
		i = 3;

		__try
		{
			ARCH::WriteMsr(IA32_RTIT_CR3_MATCH, cr3);
			i = 4;
		}
		__except (1)
		{
			cr3 = cr3 & 0xfffffffffffff000ULL;
			DbgPrint("Failed to set the actual CR3. Using a sanitized CR3: %llx\n", cr3);
			ARCH::WriteMsr(IA32_RTIT_CR3_MATCH, cr3);
		}

		ARCH::WriteMsr(IA32_RTIT_STATUS, 0);
		i = 5;
		ARCH::WriteMsr(IA32_RTIT_CTL, ctl.Value);
		i = 6;


		s.Value = ARCH::ReadMsr(IA32_RTIT_STATUS);
		i = 7;
		if (s.Bits.Error)
			DbgPrint("Setup for cpu %d failed\n", cpuIdx_);
		else
			DbgPrint("Setup for cpu %d succesful\n", cpuIdx_);
		return 0;
	}
	__except (1)
	{
		DbgPrint("ctl.Value=%p\n", ctl.Value);
		DbgPrint("CR3=%p\n", cr3);
		DbgPrint("faile at i =%u\n", i);
		return i;
	}
	
}
unsigned long long CPTConfiger::GetProcessCr3(unsigned int pid)
{
	//return ARCH::GetProcessCr3(pid);
	pid = pid;
	return cr3_;
}
bool CPTConfiger::IsPtRunning()
{
	RTIT_CTL ctl;
	ctl.Value = ARCH::ReadMsr(IA32_RTIT_CTL);
	return ctl.Bits.TraceEn == 1;
}

void CPTConfiger::StopPt()
{
	RTIT_CTL ctl;
	ctl.Value = 0;
	__try
	{
		ARCH::WriteMsr(IA32_RTIT_CTL, 0); //disable packet generation
		ARCH::WriteMsr(IA32_RTIT_STATUS, 0);
	}
	__except (1)
	{
		DbgPrint("WriteMsr IA32_RTIT_CR3_MATCH failed!");
	}
}
CPTConfiger::~CPTConfiger()
{

}

int CPTConfiger::SwapToPA()
{
	if (IsPtRunning()) {
		StopPt();
	}
	curToPAIdx_ ^= 1;
	return SetupPtCtl();
}




