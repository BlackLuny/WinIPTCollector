#pragma once
#include "CNotifier.h"

void OnPMI(int idx, void* info);
void ClearAllNotifier();
void AddNotifier(CNotifier* notifier);
void GetOutputAddress(unsigned long long* address, unsigned int& size, unsigned int& bufferLen);


