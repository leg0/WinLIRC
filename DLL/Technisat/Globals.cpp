#include "Globals.h"
#include "../Common/LircDefines.h"

HANDLE	threadExitEvent	= nullptr;
HANDLE	dataReadyEvent	= nullptr;

Receive *receive = nullptr;
Settings settings;

CRITICAL_SECTION criticalSection;

struct mytimeval start,end,last;

ir_code irCode = 0;
