#include "ARCH.h"
#include "CToPAConstructor.h"
#include "PTDef.h"

struct TblItem {
	int idx;
	unsigned long long address;
};
int CToPAConstructor::GetSizeEncodeValue(unsigned int size)
{
	if (size & 0xfff) {
		// 非4k对其
		size = GetSizeAligned(size);
	}
	size >>= 12; // 先除4k
	// 看1在第几位
	int pos = 0;
	do {
		if (size & 1) {
			return pos;
		}
		pos++;
	} while (size>>=1);
	return 0;
}

unsigned int CToPAConstructor::GetSizeAligned(unsigned int size)
{
	unsigned int val = 4096; // 最小4k, 最大128M
	while (val < size && val < 128 * 1024 * 1024) {
		val <<= 1;
	}
	return val;
}


unsigned int CToPAConstructor::GetTableNum(unsigned int bufferSize)
{
	return bufferSize / (511 * 4096) + 1;
}

unsigned int CToPAConstructor::GetTableSize(unsigned int bufferSize)
{
	return GetTableNum(bufferSize) * 4096; // 4k = 一个表 = 8 * (511 + 1) = 511 ouput region + 1 nextTablePointer
}

CToPAConstructor::~CToPAConstructor()
{
	DeconstructToPA();
}

RTL_GENERIC_COMPARE_RESULTS
NTAPI
CompareTblItem(
	_In_ struct _RTL_GENERIC_TABLE* Table,
	_In_ PVOID FirstStruct,
	_In_ PVOID SecondStruct
)
{
	Table = Table;
	auto first = (TblItem*)FirstStruct;
	auto second = (TblItem*)SecondStruct;
	if (first->address > second->address) {
		return GenericGreaterThan;
	}
	else if (first->address < second->address) {
		return GenericLessThan;
	}
	else {
		return GenericEqual;
	}
}
PVOID
NTAPI
AllocateRoutine(
	_In_ struct _RTL_GENERIC_TABLE* Table,
	_In_ CLONG ByteSize
)
{
	Table = Table;
	return ExAllocatePool(NonPagedPool, ByteSize);
}

VOID
NTAPI
FreeRoutine(
	_In_ struct _RTL_GENERIC_TABLE* Table,
	_In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
)
{
	Table = Table;
	ExFreePool(Buffer);
}

int CToPAConstructor::GetTableIdx(unsigned long long address)
{
	TblItem item;
	item.address = address;
	TblItem *rst = (TblItem *)RtlLookupElementGenericTable(&tbl_, &item);
	if (rst != nullptr) {
		return rst->idx;
	}
	return -1;
}
void CToPAConstructor::DeconstructToPA()
{
	if (topa_.outputAddr == nullptr) {
		return;
	}
	TblItem* li = nullptr;
	do {
		li = (TblItem*)RtlGetElementGenericTable(&tbl_, 0);
		if (li == nullptr) {
			break;
		}
		RtlDeleteElementGenericTable(&tbl_, li);
	} while (1);
	ARCH::ReleaseNonPagedMemory(topa_.outputAddr);
	ARCH::ReleaseNonPagedMemory(topa_.tableAddr);
	topa_.outputAddr = nullptr;
	topa_.tableAddr = nullptr;
}

unsigned int CToPAConstructor::ConstructTbl(unsigned int bufferSize, ToPAEntryStru* &tableAddr, char* &ouputAddr, int &tblIdx)
{
	RtlInitializeGenericTable(&tbl_, CompareTblItem, AllocateRoutine, FreeRoutine, nullptr);
	bufferSize &= (~0xfff);
	auto tableSize = GetTableSize(bufferSize);  // 一张大表的size
	tableAddr = (ToPAEntryStru*)(ARCH::AllocNonPagedMemory(tableSize));
	ouputAddr = (char*)(ARCH::AllocNonPagedMemory(bufferSize));
	memset(ouputAddr, 0xff, bufferSize);
	auto endOutputAddr = ouputAddr + bufferSize;
	char* curOutputAddr = ouputAddr;
	int idx = 0;
	TblItem item;
	item.idx = 0;
	item.address = ARCH::GetPhyAddress(tableAddr).QuadPart;
	RtlInsertElementGenericTable(&tbl_, &item, sizeof(item), nullptr);  // 记录下表的地址
	while (curOutputAddr < endOutputAddr) {
		auto curOutPutAddrPhyAddr = ARCH::GetPhyAddress(curOutputAddr);
		auto& curTableItem = tableAddr[idx];
		curTableItem.value = 0;
		if ((idx + 1) % 512 == 0) {
			// 最后一张表
			auto nextTableAddr = &tableAddr[idx + 1];
			curTableItem.Bits.END = 1; // 指向下一张小表
			curTableItem.Bits.INT = 0; // 不产生中断
			curTableItem.Bits.STOP = 0; // 不停止
			curTableItem.Bits.Size = 0; // ignore
			curTableItem.Bits.PhyAddr = (ARCH::GetPhyAddress(nextTableAddr).QuadPart >> 12);

			item.idx++;
			item.address = ARCH::GetPhyAddress(nextTableAddr).QuadPart;
			RtlInsertElementGenericTable(&tbl_, &item, sizeof(item), nullptr);  // 记录下表的地址
		}
		else {
			curTableItem.Bits.PhyAddr = (curOutPutAddrPhyAddr.QuadPart) >> 12;
			curTableItem.Bits.END = 0;
			curTableItem.Bits.INT = 0;
			curTableItem.Bits.STOP = 0;
			curTableItem.Bits.Size = 0;
			curOutputAddr += 4096;
		}

		idx++;

	}
	tblIdx = idx;
	return (unsigned int)(curOutputAddr - ouputAddr);
}
int CToPAConstructor::ConstructToPA(unsigned int bufferSize)
{
	ToPAEntryStru* tableAddr = nullptr;
	char* ouputAddr = nullptr;
	int idx = 0;
	auto rstSize = ConstructTbl(bufferSize, tableAddr, ouputAddr, idx);
	idx--;
    tableAddr[idx].Bits.PhyAddr = (ARCH::GetPhyAddress(tableAddr).QuadPart >> 12);  // 指向第一张表，循
	tableAddr[idx].Bits.END = 1;
	if ((idx + 1) % 512 != 0) {
		// 不是最后一张表，最后一张表被用来end了，需要减去相应的长度
		rstSize -= 4096;
	}
	topa_.outputAddr = ouputAddr;
	topa_.tableAddr = tableAddr;
	return rstSize;
}