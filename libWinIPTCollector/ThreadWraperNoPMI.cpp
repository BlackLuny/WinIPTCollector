#include "ThreadWraperNoPMI.h"
#include <iostream>
#include <fstream>
#include <string>
std::string g_outPathNoPMI = "X:\\";

unsigned int FetchData(ThreadWraperNoPMI *inst, void *outBuffer, unsigned int outBufferLen)
{
	auto origBuff = inst->GetOutBuff();
	auto origBuffLen = inst->GetBufferLen();
	auto pos = inst->GetBufferPos();
	auto rstSize = 0;
	// 从头开始测试
	for (unsigned int i = 0; i < outBufferLen; i++) {
		bool isValid = false;
		for (auto j = 0; j < 10; ++j) {
			auto curOffset = (pos + i + j) % origBuffLen;
			auto cur = *((unsigned char*)origBuff + curOffset);
			if (cur != 0xFF) {
				isValid = true;
				break;
			}
		}
		if (!isValid) {
			break;
		}
		auto offset = (i + pos) % origBuffLen;
		*((char*)outBuffer + i) = *((char*)origBuff + offset);
		rstSize++;
		*((unsigned char*)origBuff + offset) = 0xFF;
	}
	inst->SetBufferPos((pos + rstSize) % origBuffLen);
	return rstSize;
}
void ThreadWraperNoPMI::Run(std::vector<ThreadWraperNoPMI*> *instans)
{
	std::vector<std::ofstream *> allFile;
	std::vector<char *> tmpBuffs;
	unsigned int maxLen = 16 * 1024 * 1024;
	char outPutFile[256] = { 0 };
	for (auto i = 0; i < instans->size(); ++i) {
		std::string outPath = g_outPathNoPMI + "%d.dat";
		snprintf(outPutFile, 256, outPath.c_str(), i);
		allFile.push_back(new std::ofstream(outPutFile, std::ios::out | std::ios::binary));
		auto buff = new char[maxLen];
		//memset(buff, 0xff, maxLen);
		tmpBuffs.push_back(buff);
	}
	while (1) {
		int loseNum = 0;
		for (auto i = 0; i < instans->size(); ++i) {
			auto curInst = (*instans)[i];
			auto tmpBuff = tmpBuffs[i];
			auto rstSize = FetchData(curInst, tmpBuff, maxLen);
			if (rstSize > 0) {
				allFile[i]->write(tmpBuff, rstSize);
			}
			else {
				loseNum++;
			}
		}
		if (loseNum == instans->size()) {
			//break;
		}
	}
	for (auto i = 0; i < instans->size(); ++i) {
		allFile[i]->close();
	}
	std::cout << "returned\n";
}