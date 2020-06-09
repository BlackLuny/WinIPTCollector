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
		// Setup output buffer map to r3
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
	}
	__except(1){
		DbgPrint("cpu %d SetParam fail %u\n", idx);
	}
	
}