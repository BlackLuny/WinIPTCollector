#include "ProceessUtils.h"
#include "BlackBone\Process\Process.h"
#include "BlackBone\Patterns\PatternSearch.h"
#include "BlackBone\DriverControl\DriverControl.h"
#include <iostream>
#include "../common/ToolFunc.h"
#include <map>
blackbone::Process p;
bool GetProcessModulesInfo(unsigned int pid, std::vector<ModuleInfo>& moduleInfo)
{
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