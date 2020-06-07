#pragma once
#include <ntifs.h>
#include "CPTConfiger.h"
class CNotifier
{
private:
	struct ConfigInfo {
		//unsigned long outPutBuffLen_ = 0;
		PMDL outPutBuffMdl_ = nullptr; // 用于映射成用户态地址
		unsigned long long userOutPutAddr_ = 0; // 用户访问的OutpuBuff虚拟地址
		unsigned int outPutBufferLen_ = 0;
	};
public:
	CNotifier() {
	}
	~CNotifier() {
		__try {
			auto bufferNum = 1;
			for (auto i = 0; i < bufferNum; ++i) {
				auto& curConfig = buffs_[i];
				MmUnmapLockedPages((PVOID)curConfig.userOutPutAddr_, curConfig.outPutBuffMdl_);
				IoFreeMdl(curConfig.outPutBuffMdl_);
			}
			MmUnmapLockedPages((PVOID)userTransDataAddr_, transDataBuffMdl_);
			IoFreeMdl(transDataBuffMdl_);
			ExFreePool(transDataBuff_);
		}
		__except(1) {
			DbgPrint("~CNotifier failed");
		}

	}
	int GetIdx() {
		return idx_;
	}
	void SetParam(int idx, CPTConfiger * configer);
	unsigned long long GetUserTransBuff()
	{
		return userTransDataAddr_;
	}
	void *GetSysTransBuff()
	{
		return transDataBuff_;
	}
	unsigned long long GetUserOutputBuff(int idx)
	{
		return buffs_[idx].userOutPutAddr_;
	}
	unsigned int GetOutBufferLen(int idx)
	{
		return buffs_[idx].outPutBufferLen_;
	}
	void OnNotify(void* param);
private:
	int idx_ = -1;
	KDPC dpc_;
	ConfigInfo buffs_[2]; // 
	CPTConfiger* configer_; // 对应的配置器
	void* transDataBuff_ = nullptr; // 给用户传递buff信息的地址，告诉用户在哪儿去读取数据
	PMDL transDataBuffMdl_ = nullptr; // mdl
	unsigned long long userTransDataAddr_ = 0; // 用户访问的
};

