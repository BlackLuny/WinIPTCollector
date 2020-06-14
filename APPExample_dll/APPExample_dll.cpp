#include <iostream>
#include "../libWinIPTCollector_dll/framework.h"
#include "../share/common/ToolFunc.h"

#pragma comment (lib,"libWinIPTCollector_dll.lib")

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "APPExample_dll.exe [Process Name] [Output path director]";
		return 0;
	}

	// Set data ouput path
	SetOutPutPathDLL(argv[2]);
	std::string myProcessName = argv[0];
	unsigned int myPid = GetPid(myProcessName);

	// Set collector process
	SetupServerPidDLL(myPid);

	// Set target process
	const char* processName = argv[1];
	CollectIPTByProcessNameDLL(processName);
}