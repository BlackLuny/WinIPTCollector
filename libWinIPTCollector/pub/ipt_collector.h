#pragma once
extern "C" int CollectIPTByProcessName(const char *procName);
extern "C" int CollectIPTByPID(unsigned int pid);
extern "C" void SetupServerPid(unsigned int serverPid);
extern "C" void SetOutPutPath(const char *path);