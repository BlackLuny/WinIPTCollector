// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "framework.h"
#include "../libWinIPTCollector/pub/ipt_collector.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

int CollectIPTByProcessNameDLL(const char* procName)
{
    return CollectIPTByProcessName(procName);
}

int CollectIPTByPidDLL(unsigned int pid)
{
    return CollectIPTByPID(pid);
}

void SetupServerPidDLL(unsigned int serverPid)
{
    return SetupServerPid(serverPid);
}

void SetOutPutPathDLL(const char* path)
{
    return SetOutPutPath(path);
}

