#include "Globals.h"
#include "../Common/LircDefines.h"

HANDLE	threadExitEvent	= NULL;
HANDLE	dataReadyEvent	= NULL;

Receive *receive = NULL;
Settings settings;

CRITICAL_SECTION criticalSection;

struct mytimeval start,end,last;

ir_code irCode = 0;
