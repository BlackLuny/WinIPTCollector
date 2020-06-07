#pragma once
#include "ARCH.h"
#include "PTDef.h"
struct ToPAInfo {
	void* tableAddr = nullptr;
	void* outputAddr = nullptr;
};

class CToPAConstructor
{
public:
	CToPAConstructor() {}
	~CToPAConstructor();
	int ConstructToPA(unsigned int bufferSize);
	void DeconstructToPA();
	int GetTableIdx(unsigned long long address);
	void* GetOutputBuff()
	{
		return topa_.outputAddr;
	}
	void* GetToPAHeader() {
		return topa_.tableAddr;
	}
private:
	ToPAInfo topa_;
	RTL_GENERIC_TABLE tbl_;
	unsigned int ConstructTbl(unsigned int bufferSize, ToPAEntryStru*& tableAddr, char*& ouputAddr, int& tblIdx);
	int GetSizeEncodeValue(unsigned int size);
	unsigned int GetSizeAligned(unsigned int size);
	unsigned int GetTableSize(unsigned int bufferSize);
	unsigned int GetTableNum(unsigned int bufferSize);
};

