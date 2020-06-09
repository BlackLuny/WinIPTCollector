#include <iostream>
#include "../libWinIPTCollector/pub/ipt_collector.h"
#include "../share/common/ToolFunc.h"
int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Please input process name!\n";
		return 0;
	}

	// Set data ouput path
	SetOutPutPath("X:\\");
	std::string myProcessName = argv[0];
	unsigned int myPid = GetPid(myProcessName);

	// Set collector process
	SetupServerPid(myPid);

	// Set target process
	const char * processName = argv[1];
	CollectIPTByProcessName(processName);
}

