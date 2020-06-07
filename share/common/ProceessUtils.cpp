#include "ProceessUtils.h"
#include "BlackBone\Process\Process.h"
#include "BlackBone\Patterns\PatternSearch.h"
#include "BlackBone\DriverControl\DriverControl.h"
#include <iostream>
#include "../common/ToolFunc.h"
#include <map>
std::map<unsigned long long, unsigned long long> g_regions;
blackbone::Process p;
bool GetProcessModulesInfo(unsigned int pid, std::vector<ModuleInfo>& moduleInfo)
{
	if (g_regions.size() == 0) {
		std::vector<MemRegionInfo> regions;
		EnumExecuteablePages(pid, regions);
		for (auto& x : regions) {
			g_regions[x.baseAddr] = g_regions[x.baseAddr] + x.size;
		}
		/*for (auto& x : g_regions) {
			std::cout << "addr: " << (void*)x.first << " size: " << (void*)x.second << std::endl;
		}*/

	}
	moduleInfo.clear();
	if (!p.valid()) {
		p.Attach(pid);
	}
	if (!p.valid()) {
		std::cout << "Attach failed!\n";
		return false;
	}
	auto& drv = blackbone::DriverControl::Instance();
	auto status = drv.EnsureLoaded();
	if (!NT_SUCCESS(status)) {
		std::cout << "load bb driver failed!\n";
		return false;
	}
	status = drv.PromoteHandle(GetCurrentProcessId(), p.core().handle(), PROCESS_ALL_ACCESS);
	if (!NT_SUCCESS(status)) {
		std::cout << "change handle access failed!\n";
		return false;
	}
	auto allModules = p.modules().GetAllModules();
	for (auto& x : allModules) {
		auto data = x.second;
		if (data == nullptr) {
			continue;
		}
		ModuleInfo info;
		info.name = WStringToString(data->name);
		info.base = data->baseAddress;
		info.size = data->size;
		moduleInfo.push_back(info);
	}
	return true;
}

bool ReadProcessMemoryBB(
	unsigned int pid, unsigned long long address, unsigned int len,
	void* outputBuff, unsigned int outputBuffSize, unsigned int* outputSize)
{
	if (outputSize == nullptr || outputBuff == nullptr) {
		return false;
	}
	if (!p.valid()) {
		p.Attach(pid);
	}
	if (!p.valid()) {
		std::cout << "Attach failed!\n";
		return false;
	}
	auto& drv = blackbone::DriverControl::Instance();
	auto status = drv.EnsureLoaded();
	if (!NT_SUCCESS(status)) {
		std::cout << "load bb driver failed!\n";
		return false;
	}

	//static unsigned long long lastRegionOk = 0;
	bool isOk = false;
	//if ((address & (~0xFFF)) == lastRegionOk) {
	//	isOk = true;
	//}
	//else {
	for (auto& x : g_regions) {
		if (x.first <= address && address <= x.first + x.second) {
			isOk = true;
			//lastRegionOk = x.baseAddr;
			break;
		}
	}
	//}
	if (!isOk) {
		*outputSize = 0;
		return false;
	}
	if (!NT_SUCCESS(drv.ReadMem(pid, address, len, outputBuff))) {
		std::cout << "Read memory: " << (void*)address << " size: " << len << " failed!\n";
		return false;
	}
	/*
	if (!NT_SUCCESS(p.memory().Read(address, len, outputBuff))) {
		std::cout << "Read memory: " << (void *)address << " size: " << len << " failed!\n";
		return false;
	}*/
	*outputSize = len;
	return true;
}


void EnumExecuteablePages(unsigned int pid, std::vector< MemRegionInfo>& regionInfo)
{
	regionInfo.clear();
	auto& drv = blackbone::DriverControl::Instance();
	auto status = drv.EnsureLoaded();
	if (!NT_SUCCESS(status)) {
		std::cout << "load bb driver failed!\n";
		return;
	}
	std::vector< MEMORY_BASIC_INFORMATION64> regionsInfo;
	if (!NT_SUCCESS(drv.EnumMemoryRegions(pid, regionsInfo))) {
		std::cout << "EnumMemoryRegions failed\n";
		return;
	}
	for (auto& x : regionsInfo) {
		/*if ((x.Protect == PAGE_EXECUTE) || (x.Protect == PAGE_EXECUTE_READ) || (x.Protect == PAGE_EXECUTE_READWRITE)
			|| (x.Protect == PAGE_EXECUTE_WRITECOPY)) {
			if (x.Type == MEM_IMAGE && x.State == MEM_COMMIT)
				std::cout << "Base: " << (void*)x.AllocationBase << " size: " << x.RegionSize
				<< " Protect: " << x.Protect
				<< std::endl;
		}*/
		MemRegionInfo info;
		info.baseAddr = x.AllocationBase;
		info.size = x.RegionSize;
		info.protect = x.Protect;
		info.state = x.State;
		info.type = x.Type;
		regionInfo.emplace_back(info);
	}
}

void WriteMemoryBB(unsigned int pid, unsigned long long base, unsigned int size, void* buff)
{
	if (!p.valid()) {
		p.Attach(pid);
	}
	if (!p.valid()) {
		std::cout << "Attach failed!\n";
		return;
	}
	auto& drv = blackbone::DriverControl::Instance();
	auto status = drv.EnsureLoaded();
	if (!NT_SUCCESS(status)) {
		std::cout << "load bb driver failed!\n";
		return;
	}
	status = drv.ProtectMem(pid, base, size, PAGE_EXECUTE_READWRITE);
	if (!NT_SUCCESS(status)) {
		std::cout << "ProtectMem memory: " << (void*)base << " size: " << size << " failed!\n";
		return;
	}
	status = drv.WriteMem(pid, base, size, buff);
	if (!NT_SUCCESS(status)) {
		std::cout << "write memory: " << (void*)base << " size: " << size << " failed!\n";
		return;
	}
}

bool g_mapped = false;
bool MapProcessMemory(unsigned int pid)
{
	if (!p.valid()) {
		p.Attach(pid);
	}
	if (!p.valid()) {
		std::cout << "Attach failed!\n";
		return false;
	}
	if (!g_mapped) {
		p.memory().Map(true);
		p.memory().SetupHook(blackbone::RemoteMemory::MemVirtualAlloc);
		p.memory().SetupHook(blackbone::RemoteMemory::MemVirtualFree);
		p.memory().SetupHook(blackbone::RemoteMemory::MemMapSection);
		p.memory().SetupHook(blackbone::RemoteMemory::MemUnmapSection);
		g_mapped = true;
	}
}
bool UnMapProcessMemory()
{
	if (!p.valid()) {
		std::cout << "Attach failed!\n";
		return false;
	}
	if (g_mapped) {
		p.memory().reset();
		g_mapped = false;
	}
}
bool ReadMemoryByMap(unsigned int pid, unsigned long long address, unsigned int len,
	void* outputBuff, unsigned int outputBuffSize, unsigned int* outputSize)
{
	if (outputSize == nullptr || outputBuff == nullptr) {
		return false;
	}
	if (!p.valid()) {
		p.Attach(pid);
	}
	if (!p.valid()) {
		std::cout << "Attach failed!\n";
		return false;
	}
	auto translated = p.memory().TranslateAddress(address, false);
	if (translated == 0) {
		std::cout << "Read memory: " << (void*)address << " size: " << len << " failed!\n";
		return false;
	}
	memcpy(outputBuff, (void*)translated, len);
	*outputSize = len;
	return true;
}

bool ReadMemoryByMapNew(unsigned int pid, unsigned long long address, unsigned int len,
	void*& rstAddr)
{
	if (!p.valid()) {
		p.Attach(pid);
	}
	if (!p.valid()) {
		std::cout << "Attach failed!\n";
		return false;
	}
	auto translated = p.memory().TranslateAddress(address, false);
	if (translated == 0) {
		std::cout << "Read memory: " << (void*)address << " size: " << len << " failed!\n";
		return false;
	}
	rstAddr = (void*)translated;
	return true;
}

ModuleInfo QueryModuleInfo(unsigned long long addr, const std::vector<ModuleInfo>& moduleInfo)
{
	static ModuleInfo moduleInfoCache;
	if (addr >= moduleInfoCache.base && addr <= moduleInfoCache.base + moduleInfoCache.size) {
		return moduleInfoCache;
	}
	// cache miss
	for (auto& cur : moduleInfo) {
		if (addr >= cur.base && addr <= cur.base + cur.size) {
			moduleInfoCache = cur;
			return moduleInfoCache;
		}
	}
	return ModuleInfo();
}

ModuleInfo QueryModuleInfo(std::string& name, const std::vector<ModuleInfo>& moduleInfo)
{
	static ModuleInfo moduleInfoCache;
	if (name == moduleInfoCache.name) {
		return moduleInfoCache;
	}
	// cache miss
	for (auto& cur : moduleInfo) {
		if (name == cur.name) {
			moduleInfoCache = cur;
			return moduleInfoCache;
		}
	}
	return ModuleInfo();
}

void ManualLoad(unsigned int pid, const wchar_t* filePath)
{
	if (!p.valid()) {
		p.Attach(pid);
	}
	if (!p.valid()) {
		std::cout << "Attach failed!\n";
		return;
	}
	auto& drv = blackbone::DriverControl::Instance();
	auto status = drv.EnsureLoaded();
	if (!NT_SUCCESS(status)) {
		std::cout << "load bb driver failed!\n";
		return;
	}
	status = drv.PromoteHandle(GetCurrentProcessId(), p.core().handle(), PROCESS_ALL_ACCESS);
	if (!NT_SUCCESS(status)) {
		std::cout << "change handle access failed!\n";
		return;
	}
	auto r = p.mmap().MapImage(filePath);
	if (!r.success()) {
		std::cout << "Load failed!\n" << r.status << std::endl;
	}
}