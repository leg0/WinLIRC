/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 2010 Ian Curtis
 */

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
	}

}