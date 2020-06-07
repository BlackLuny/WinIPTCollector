#ifndef _ITF_H_
#define _ITF_H_

#define MAX_CPU_NUM 32
struct EnventInfo {
	int idx;
	unsigned long long eventHandle;
};
enum AddrRangeCfgMode {
	ARCM_UNUSED = 0,
	ARCM_FILEN = 1,
	ARCM_STOP = 2,
};
struct AddrRange {
	unsigned long long addrN_A;
	unsigned long long addrN_B;
	AddrRangeCfgMode cfgMode;
};
struct PtSetupInfo {
	unsigned int pid;
	unsigned int buffSize;
	unsigned int mtcFreq;
	unsigned int retCompress;
	unsigned int cycThld;
	unsigned int psbFreq;
	unsigned int cpuNum;
	AddrRange addrsCfg[4];
};

struct PtResultInfo {
	unsigned long long outputAddr;
	unsigned int dataLen;
};

struct PtRecorInfo {
	PtResultInfo* rstInfo;
};
struct PtSetupRst {
	unsigned int rst;
	unsigned int recordNum;
	PtRecorInfo recordInfo[MAX_CPU_NUM];
	unsigned int outBuffNum;
	unsigned long long outBufferInfo[MAX_CPU_NUM];
	unsigned int outBufferLen;
};

struct PtReadMsrRst {
	unsigned long long rst;
};
struct PtReadMsr {
	unsigned int msrId;
};

struct PtSetupServerPid {
	unsigned int pid;
};
struct PtSetupServerPidRsp {
	unsigned int rst;
};

struct PtReadMemoryReq {
	unsigned int pid;
	unsigned int len; // <=4096
	unsigned long long address;
};
struct PtReadMemoryRsp {
	unsigned char data[4096];
};
struct PtWriteMemoryReq {
	unsigned int pid;
	unsigned long long address;
	unsigned int len; // <=4096
	unsigned char data[4096];
};
struct PtWriteMemoryRsp {
	unsigned int rst;
	unsigned int len;
};

#endif