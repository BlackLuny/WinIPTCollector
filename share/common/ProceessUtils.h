#pragma once
#include <string>
#include <vector>
struct ModuleInfo {
	std::string name = "unknown";
	unsigned long long base = 0;
	size_t size = 0;
};
struct MemRegionInfo {
	unsigned long long baseAddr = 0;
	unsigned long long size = 0;
	unsigned long      protect = 0;
	unsigned long      state = 0;
	unsigned long      type = 0;
	MemRegionInfo() = default;
};
bool GetProcessModulesInfo(unsigned int pid, std::vector<ModuleInfo>& moduleInfo);
bool ReadProcessMemoryBB(
	unsigned int pid, unsigned long long address, unsigned int len,
	void* outputBuff, unsigned int outputBuffSize, unsigned int* outputSize);
void EnumExecuteablePages(unsigned int pid, std::vector< MemRegionInfo> &regionInfo);
void WriteMemoryBB(unsigned int pid, unsigned long long base, unsigned int size, void* buff);
bool ReadMemoryByMap(unsigned int pid, unsigned long long address, unsigned int len,
	void* outputBuff, unsigned int outputBuffSize, unsigned int* outputSize);
bool MapProcessMemory(unsigned int pid);
bool UnMapProcessMemory();
bool ReadMemoryByMapNew(unsigned int pid, unsigned long long address, unsigned int len,
	void*& rstAddr);
ModuleInfo QueryModuleInfo(unsigned long long addr, const std::vector<ModuleInfo>& moduleInfo);

ModuleInfo QueryModuleInfo(std::string& name, const std::vector<ModuleInfo>& moduleInfo);

void ManualLoad(unsigned int pid, const wchar_t* filePath);