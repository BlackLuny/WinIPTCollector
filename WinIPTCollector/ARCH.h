#pragma once
#include <ntifs.h>
#include <ntddk.h>
namespace ARCH
{
	using CpuCallBack = void (*)(void* param);
	unsigned long long GetProcessCr3(unsigned int pid);
	unsigned long long GetCR3(void);
	unsigned long long ReadMsr(unsigned long msrId);
	void WriteMsr(unsigned long msrId, unsigned long long value);
	unsigned long GetCpuCount();
	void ForEachCpu(CpuCallBack func, void *param);
	int GetCurrentCpuIdx();
	void* AllocContinueMemory(unsigned int size, unsigned int aligneSize);
	void* AllocNonPagedMemory(unsigned int size);
	PHYSICAL_ADDRESS GetPhyAddress(void* addr);
	void ReleaseContinueMemory(void* addr);
	void ReleaseNonPagedMemory(void* addr);
};

