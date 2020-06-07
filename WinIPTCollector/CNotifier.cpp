#include "CNotifier.h"
#include "ARCH.h"
#include "PTDef.h"
#include "..//share/itf/itf.h"
void CNotifier::OnNotify(void* param)
{
	param = param;
	auto cpuIdx = ARCH::GetCurrentCpuIdx();
	if (cpuIdx != idx_) {
		DbgPrint("CNotifier diff idx cpuIdx[%u] idx[%u]\n", cpuIdx, idx_);
		return;
	}
	KeSetTargetProcessorDpc(&dpc_, (CCHAR)cpuIdx);
	KeInsertQueueDpc(&dpc_, nullptr, nullptr);
}

void CNotifier::SetParam(int idx, CPTConfiger* configer)
{
	__try {
		// 设置输出buff映射
		auto bufferNum = 1;
		for (auto i = 0; i < bufferNum; ++i) {
			auto curOutPutBuff = configer->GetBuffByIdx(i);
			buffs_[i].outPutBufferLen_ = configer->GetBuffSize();
			buffs_[i].outPutBuffMdl_ = IoAllocateMdl(curOutPutBuff, configer->GetBuffSize(), false, false, nullptr);
			MmBuildMdlForNonPagedPool(buffs_[i].outPutBuffMdl_);
			buffs_[i].userOutPutAddr_ = (unsigned long long)MmMapLockedPagesSpecifyCache(buffs_[i].outPutBuffMdl_, UserMode, MmCached, nullptr, FALSE, NormalPagePriority);
		}
		idx_ = idx;
		configer_ = configer;

		// 映射信息传递buff
		transDataBuff_ = ExAllocatePool(NonPagedPool, 4096);
		transDataBuffMdl_ = IoAllocateMdl(transDataBuff_, 4096, false, false, nullptr);
		MmBuildMdlForNonPagedPool(transDataBuffMdl_);
		userTransDataAddr_ = (unsigned long long)MmMapLockedPagesSpecifyCache(transDataBuffMdl_, UserMode, MmCached, nullptr, FALSE, NormalPagePriority);
	}
	__except(1){
		DbgPrint("cpu %d SetParam fail %u\n", idx);
	}
	
}