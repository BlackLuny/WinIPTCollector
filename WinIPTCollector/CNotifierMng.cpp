#include "CNotifierMng.h"
#define MAX_NOTIFIERS 32
static int notifierNum = 0;
static CNotifier* notifiers_[MAX_NOTIFIERS];

void AddNotifier(CNotifier* notifier)
{
	if (notifierNum < MAX_NOTIFIERS) {
		notifiers_[notifierNum] = notifier;
		notifierNum++;
	}
}

void GetOutputAddress(unsigned long long* address, unsigned int& size, unsigned int&bufferLen)
{
	size = 0;
	for (auto i = 0; i < notifierNum; ++i) {
		if (notifiers_[i] != nullptr) {
			auto addr = notifiers_[i]->GetUserOutputBuff(0);
			address[size++] = addr;
			bufferLen = notifiers_[i]->GetOutBufferLen(0);
		}
	}
}
void OnPMI(int idx, void* info) {
	if (idx < notifierNum) {
		notifiers_[idx]->OnNotify(info);
	}
}

void ClearAllNotifier()
{
	for (auto i = 0; i < notifierNum; ++i) {
		if (notifiers_[i] != nullptr) {
			delete notifiers_[i];
		}
	}
	notifierNum = 0;
}
