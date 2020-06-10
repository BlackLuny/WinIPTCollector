#include <iostream>
#include "../libWinIPTCollector/pub/ipt_collector.h"
#include "../share/common/ToolFunc.h"
int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "APPExample.exe [Process Name] [Output path director]";
		return 0;
	}

	// Set data ouput path
	SetOutPutPath(argv[2]);
	std::string myProcessName = argv[0];
	unsigned int myPid = GetPid(myProcessName);

	// Set collector process
	SetupServerPid(myPid);

	// Set target process
	const char * processName = argv[1];
	CollectIPTByProcessName(processName);
}

