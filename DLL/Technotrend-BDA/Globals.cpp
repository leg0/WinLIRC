#include "Globals.h"
#include "LircDefines.h"

HANDLE	threadExitEvent	= NULL;
HANDLE	dataReadyEvent	= NULL;

Receive *receive = NULL;
Settings settings;

CRITICAL_SECTION criticalSection;

struct mytimeval start,end,last;

ir_code irCode = 0;

int gettimeofday(struct mytimeval *a, void *)
/* only accurate to milliseconds, instead of microseconds */
{
	struct _timeb tstruct;
	_ftime(&tstruct);
	
	a->tv_sec=tstruct.time;
	a->tv_usec=tstruct.millitm*1000;

	return 1;
}
