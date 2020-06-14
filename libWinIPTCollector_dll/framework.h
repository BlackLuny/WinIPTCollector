#pragma once

// Windows 头文件
#include <windows.h>

#ifdef IMPORT_DLL
#else
#define IMPORT_DLL extern "C" _declspec(dllimport)
#endif

IMPORT_DLL int CollectIPTByProcessNameDLL(const char* procName);
IMPORT_DLL int CollectIPTByPidDLL(unsigned int pid);
IMPORT_DLL void SetupServerPidDLL(unsigned int serverPid);
IMPORT_DLL void SetOutPutPathDLL(const char* path);