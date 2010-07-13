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

#include <windows.h>
#include "SendReceiveData.h"
#include "Globals.h"
#include <stdio.h>

DWORD WINAPI IRToy(void *recieveClass) {

	((SendReceiveData*)recieveClass)->threadProc();
	return 0;
}

SendReceiveData::SendReceiveData() {

	bufferStart		= 0;
	bufferEnd		= 0;
	recvDone		= 0;
	threadHandle	= NULL;

}

bool SendReceiveData::init() {

	threadHandle = CreateThread(NULL,0,IRToy,(void *)this,0,NULL);

	recvDone = 0;

	if(threadHandle) {
		return true;
	}

	return false;
}

void SendReceiveData::deinit() {

	killThread();

	//clean up other stuff
}


void SendReceiveData::threadProc() {

	receiveLoop();
}

void SendReceiveData::killThread() {

	recvDone = 1;

	if(threadHandle!=NULL) {

		//===========
		DWORD result;
		//===========

		result = 0;

		if(GetExitCodeThread(threadHandle,&result)==0) 
		{
			threadHandle = NULL;
			return;
		}

		if(result==STILL_ACTIVE)
		{
			WaitForSingleObject(threadHandle,INFINITE);
			threadHandle = NULL;
		}
	}
}

void SendReceiveData::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==NULL) evt=1;
	else evt=2;

	if(!dataReady())
	{
		ResetEvent(dataReadyEvent);
		int res;
		if(maxUSecs)
			res=WaitForMultipleObjects(evt,events,FALSE,(maxUSecs+500)/1000);
		else
			res=WaitForMultipleObjects(evt,events,FALSE,INFINITE);
		if(res==(WAIT_OBJECT_0+1))
		{
			//DEBUG("Unknown thread terminating (readdata)\n");
			ExitThread(0);
			return;
		}
	}

}

void SendReceiveData::setData(lirc_t data) {

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool SendReceiveData::dataReady() {

	if(bufferStart==bufferEnd) return false;
	
	return true;
}

bool SendReceiveData::getData(lirc_t *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

void SendReceiveData::receiveLoop() {

	while(! recvDone)
	{
		//
		//check for data here
		//

		//
		//if we have data call setData(dataValue|PULSE_BIT)
		//

		//
		//signal other thread data is ready with SetEvent(dataReadyEvent);
		//

		Sleep(100);
	}
}

//======================================================================================
// sending stuff below
//======================================================================================

int SendReceiveData::send(ir_remote *remote, ir_ncode *code, int repeats) {

	return 0;
}

