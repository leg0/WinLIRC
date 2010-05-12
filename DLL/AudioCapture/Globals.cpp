#include "Globals.h"
#include <Windows.h>
#include <stdio.h>
#include "LircDefines.h"

RecordAudio		*recordAudio	= NULL;
AnalyseAudio	*analyseAudio	= NULL;
Settings		*settings		= NULL;
HANDLE			dataReadyEvent	= NULL;
HANDLE			threadExitEvent	= NULL;

void waitTillDataIsReady(int maxUSecs) {

	//
	// if buffer start = buffer end we need to wait and read more data
	//

	if(!analyseAudio) return;

	if(!analyseAudio->dataReady()) {

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