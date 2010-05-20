#include <Windows.h>
#include "Globals.h"
#include <stdio.h>
#include "LircDefines.h"
#include <sys/timeb.h>

HANDLE	threadExitEvent	= NULL;
HANDLE	dataReadyEvent	= NULL;

CRITICAL_SECTION criticalSection;

struct mytimeval start,end,last;

void waitTillDataIsReady(int maxUSecs) {

	//
	// if buffer start = buffer end we need to wait and read more data
	//

	if(1) {

		//=====================
		HANDLE	events[2];
		int		res;
		int		numberOfEvents;
		//=====================

		res			= 0;
		events[0]	= dataReadyEvent;

		if(threadExitEvent) {
			numberOfEvents = 2;
			events[1] = threadExitEvent;
		}
		else {
			numberOfEvents = 1;
		}
		
		ResetEvent(dataReadyEvent);
		
		if(maxUSecs==0) {
			res=WaitForMultipleObjects(numberOfEvents,events,FALSE,INFINITE);
		}
		else {
			res=WaitForMultipleObjects(numberOfEvents,events,FALSE,(maxUSecs+500)/1000);
		}

		if(res==(WAIT_OBJECT_0+1)) {
			ExitThread(0);
		}


		//
		// waited for data it must have arrived so function will now return !
		//

		return;
	}

}

int gettimeofday(struct mytimeval *a, void *)
/* only accurate to milliseconds, instead of microseconds */
{
	struct _timeb tstruct;
	_ftime(&tstruct);
	
	a->tv_sec=tstruct.time;
	a->tv_usec=tstruct.millitm*1000;

	return 1;
}