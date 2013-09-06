#include <Windows.h>
#include "Globals.h"
#include <stdio.h>
#include "../Common/LircDefines.h"

HANDLE	threadExitEvent	= NULL;
HANDLE	dataReadyEvent	= NULL;

CRITICAL_SECTION criticalSection;

struct mytimeval start,end,last;
