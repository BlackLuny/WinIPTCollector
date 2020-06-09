#pragma once
#include <windows.h>
#include <vector>
#include <string>
struct PidInfo
{
	std::string pidName;
	DWORD pid;
};

#include <sstream>
#include <string>
template<class Target, class Source>
Target Convert(Source src)
{
	std::stringstream ss;
	Target rst;
	ss << src;
	ss >> rst;
	return rst;
}
void StringToWstring(std::wstring& szDst, std::string str);
std::string WStringToString(const std::wstring& wstr);
void Wchar_tToString(std::string& szDst, wchar_t* wchar);
unsigned long long Hex2Ull(std::string &&str);
void GetRuntimePidInfo(std::vector <PidInfo>& list);

DWORD GetPid(std::string& pidName);

size_t ReadMemory(HANDLE process, char* buffer, size_t size,
	unsigned long long addr);


std::string FillTailSpace(const std::string &input, int expectSize);
std::string RemoveHeadZeros(const std::string& input);