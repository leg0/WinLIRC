#include <Windows.h>
#include "Globals.h"
#include <stdio.h>
#include "../Common/LircDefines.h"

HANDLE	threadExitEvent	= nullptr;
HANDLE	dataReadyEvent	= nullptr;

CRITICAL_SECTION criticalSection;

std::chrono::steady_clock::time_point start, end, last;
