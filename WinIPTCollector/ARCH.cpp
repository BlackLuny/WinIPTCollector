#include "ARCH.h"
#include <intrin.h>

namespace ARCH {
	unsigned long long GetCR3(void)
	{
		return __readcr3();
	}
	unsigned long long ReadMsr(unsigned long msrId)
	{
		return __readmsr(msrId);
	}
	void WriteMsr(unsigned long msrId, unsigned long long value)
	{
		__writemsr(msrId, value);
	}

	unsigned long GetCpuCount()
	{
		return KeGetCurrentProcessorNumber();
	}
	unsigned long long GetProcessCr3(unsigned int pid)
	{
		UINT64 CurrentCr3 = 0;
		PEPROCESS proceses = nullptr;
		if (PsLookupProcessByProcessId((PVOID)pid, &proceses) == STATUS_SUCCESS)
		{
			//todo add specific windows version checks and hardcode offsets/ or use scans
			if (GetCR3() & 0xfff)
			{
				DbgPrint("Split kernel/usermode pages\n");
				//uses supervisor/usermode pagemaps			
				CurrentCr3 = *(UINT64*)((UINT_PTR)proceses + 0x278);
				if ((CurrentCr3 & 0xfffffffffffff000ULL) == 0)
				{
					DbgPrint("No usermode CR3\n");
					CurrentCr3 = *(UINT64*)((UINT_PTR)proceses + 0x28);
				}

				DbgPrint("CurrentCR3=%llx\n", CurrentCr3);
			}
			else
			{
				KAPC_STATE apc_state;
				RtlZeroMemory(&apc_state, sizeof(apc_state));
				__try
				{
					KeStackAttachProcess((PRKPROCESS)proceses, &apc_state);
					CurrentCr3 = GetCR3();
					KeUnstackDetachProcess(&apc_state);
				}
				__except (1)
				{
					DbgPrint("Failure getting CR3 for this process");
					ObDereferenceObject(proceses);
					return 0;
				}
			}
			ObDereferenceObject(proceses);
		}
		else
		{
			DbgPrint("Failure getting the EProcess for pid %d", pid);
			return 0;
		}
		return CurrentCr3;
	}


	VOID DpcDelegate(
			_In_ struct _KDPC* Dpc,
			_In_opt_ PVOID DeferredContext,
			_In_opt_ PVOID SystemArgument1,
			_In_opt_ PVOID SystemArgument2
		)
	{
		Dpc = Dpc;
		SystemArgument2 = SystemArgument2;
		auto func = (CpuCallBack)DeferredContext;
		func(SystemArgument1);
	}
	void ForEachCpu(CpuCallBack func, void* param)
	{
		CCHAR cpunr;
		KAFFINITY cpus;
		ULONG cpucount;
		PKDPC dpc;
		int dpcnr;


		//KeIpiGenericCall is not present in xp

		//count cpus first KeQueryActiveProcessorCount is not present in xp)
		cpucount = 0;
		cpus = KeQueryActiveProcessors();
		while (cpus)
		{
			if (cpus % 2)
				cpucount++;

			cpus = cpus / 2;
		}

		dpc = (PKDPC)ExAllocatePool(NonPagedPool, sizeof(KDPC) * cpucount);



		cpus = KeQueryActiveProcessors();
		cpunr = 0;
		dpcnr = 0;
		while (cpus)
		{
			if (cpus % 2)
			{
				//bit is set

				//DbgPrint("Calling dpc routine for cpunr %d (dpc=%p)\n", cpunr, &dpc[dpcnr]);
				KeInitializeDpc(&dpc[dpcnr], DpcDelegate, func);
				KeSetTargetProcessorDpc(&dpc[dpcnr], cpunr);
				KeInsertQueueDpc(&dpc[dpcnr], param, nullptr);
				KeFlushQueuedDpcs();
				dpcnr++;
			}

			cpus = cpus / 2;
			cpunr++;
		}


		ExFreePool(dpc);
	}

	int GetCurrentCpuIdx()
	{
		return KeGetCurrentProcessorNumber();
	}

	void* AllocContinueMemory(unsigned int size, unsigned int aligneSize)
	{
		PHYSICAL_ADDRESS lowAddr, highAddr, boundary;
		lowAddr.QuadPart = 0;
		highAddr.QuadPart = 0xFFFFFFFFFFFFFFFFULL;
		boundary.QuadPart = aligneSize;
		return MmAllocateContiguousMemorySpecifyCache(size, lowAddr, highAddr, boundary, MmCached);
	}
	void* AllocNonPagedMemory(unsigned int size)
	{
		return ExAllocatePool(NonPagedPool, size);
	}

	void ReleaseContinueMemory(void* addr)
	{
		MmFreeContiguousMemory(addr);
	}

	void ReleaseNonPagedMemory(void* addr)
	{
		ExFreePool(addr);
	}

	PHYSICAL_ADDRESS GetPhyAddress(void* addr)
	{
		return MmGetPhysicalAddress(addr);
	}
};
