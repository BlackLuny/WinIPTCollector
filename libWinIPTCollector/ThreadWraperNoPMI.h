#pragma once
#include <vector>
class ThreadWraperNoPMI
{
public:
	static void Run(std::vector<ThreadWraperNoPMI*>* instans);
	void SetOutBuff(void* buff) {
		outBuff_ = buff;
	}
	void* GetOutBuff() {
		return outBuff_;
	}
	unsigned int GetBufferLen() {
		return bufferLen_;
	}
	void SetBufferLen(unsigned int len) {
		bufferLen_ = len;
	}
	void SetBufferPos(unsigned int pos) {
		pos_ = pos;
	}
	unsigned int GetBufferPos() {
		return pos_;
	}
private:
	void *outBuff_ = nullptr;
	unsigned int bufferLen_ = 0;
	unsigned int pos_ = 0;
};

