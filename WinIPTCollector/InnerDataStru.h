#pragma once
#include "..//share/itf/itf.h"
struct PtSetupInfoInner {
	PtSetupInfo setupInfo;
	unsigned long long cr3;
	int setupRst;
};

