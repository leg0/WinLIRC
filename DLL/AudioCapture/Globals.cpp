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
#include <chrono>

RecordAudio		*recordAudio	= nullptr;
AnalyseAudio	*analyseAudio	= nullptr;
Settings		*settings		= nullptr;
HANDLE			dataReadyEvent	= nullptr;
HANDLE			threadExitEvent	= nullptr;

bool waitTillDataIsReady(std::chrono::microseconds timeout) {

	//
	// if buffer start = buffer end we need to wait and read more data
	//

	if(!analyseAudio) return false;

	if(!analyseAudio->dataReady()) {

		//=====================
		HANDLE	events[2];
		int		numberOfEvents;
		//=====================

		events[0]	= dataReadyEvent;

		if(threadExitEvent) {
			numberOfEvents = 2;
			events[1] = threadExitEvent;
		}
		else {
			numberOfEvents = 1;
		}
		
		ResetEvent(dataReadyEvent);
		
		using namespace std::chrono;
		DWORD const dwTimeout = timeout > 0us
			? duration_cast<milliseconds>(timeout + 500us).count()
			: INFINITE;
		DWORD const res = ::WaitForMultipleObjects(2, events, false, dwTimeout);

		if(res==(WAIT_OBJECT_0+1)) {
			return false;
		}
	}

	return true;

}