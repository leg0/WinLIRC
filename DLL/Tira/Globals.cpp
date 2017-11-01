#include <Windows.h>
#include "Globals.h"
#include <stdio.h>
#include "../Common/LircDefines.h"

HANDLE	threadExitEvent	= nullptr;
HANDLE	dataReadyEvent	= nullptr;

CRITICAL_SECTION criticalSection;

struct mytimeval start,end,last;
