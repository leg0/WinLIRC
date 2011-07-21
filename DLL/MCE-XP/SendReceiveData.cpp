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
#include "Send.h"
#include <tchar.h>

DWORD WINAPI MCEthread(void *recieveClass) {

	((SendReceiveData*)recieveClass)->threadProc();
	return 0;
}

SendReceiveData::SendReceiveData() {

	deviceHandle	= NULL;
	exitEvent		= NULL;

	QueryPerformanceFrequency(&frequency);
}

bool SendReceiveData::init() {

	//====================
	std::wstring pipeName;
	//====================

	space = PULSE_MASK;

	QueryPerformanceCounter(&lastTime);

	if(irDeviceList.get().empty()) return false;	//no hardware

	pipeName = irDeviceList.get().begin()->c_str();

	deviceHandle = CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE,0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if(deviceHandle==INVALID_HANDLE_VALUE) {
		//printf("invalid device handle\n");
		return false;
	}

	threadHandle = CreateThread(NULL,0,MCEthread,(void *)this,0,NULL);

	if(!threadHandle) {
		return false;
	}

	return true;
}

void SendReceiveData::deinit() {

	killThread();
	
	if(exitEvent) {
		CloseHandle(exitEvent);
		exitEvent = NULL;
	}

	if(deviceHandle) {
		CloseHandle(deviceHandle);
		deviceHandle = NULL;
	}
}


void SendReceiveData::threadProc() {

	//========================
	OVERLAPPED	overlappedRead;
	HANDLE		events[2];
	DWORD		result;
	UCHAR		buffer[256];
	DWORD		bytesRead;
	//========================

	memset(&overlappedRead,0,sizeof(OVERLAPPED));

	overlappedRead.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	exitEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

	events[0] = overlappedRead.hEvent;
	events[1] = exitEvent;

	while(true) {

		ReadFile(deviceHandle, buffer, sizeof(buffer), &bytesRead, &overlappedRead);

		result = WaitForMultipleObjects(2,events,FALSE,INFINITE);

		if(result==(WAIT_OBJECT_0)) 
		{
			//======================
			UCHAR	*bufferPointer;
			//======================

			GetOverlappedResult(deviceHandle, &overlappedRead, &bytesRead, FALSE);

			//printf("bytesRead %i\n",bytesRead);

			bufferPointer = buffer;

			while(bufferPointer < bufferPointer+bytesRead) {

				if(*bufferPointer >= 0x81 && *bufferPointer <= 0x9E) {

					//===========
					UCHAR length;
					//===========

					length = *bufferPointer & 0x7F;

					decodeRaw(bufferPointer);

					bufferPointer += length + 1;

					break;
				}
				else if (*bufferPointer == 0x9F) {

					bufferPointer += 8;
					space += 100000;	//fake a space value, sigh :p
				}

			}

			//printf("\n");


			SetEvent(dataReadyEvent);
		}

		//if(result==(WAIT_OBJECT_0+1))
		else 
		{
			CancelIo(deviceHandle);
			//printf("leaving thread \n");
			break;
		}
	}

	//printf("exited thread\n");
}

void SendReceiveData::killThread() {

	if(exitEvent) {
		SetEvent(exitEvent);
	}

	if(threadHandle!=NULL) {

		//===========
		DWORD result;
		//===========

		result = 0;

		if(GetExitCodeThread(threadHandle,&result)==0) 
		{
			CloseHandle(threadHandle);
			threadHandle = NULL;
			return;
		}

		if(result==STILL_ACTIVE)
		{
			WaitForSingleObject(threadHandle,INFINITE);
		}

		CloseHandle(threadHandle);
		threadHandle = NULL;
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

void SendReceiveData::decodeRaw(UCHAR *in) {

	//================
	UCHAR	length;
	lirc_t	lastValue;
	lirc_t	value;
	//================

	length		= in[0] & 0x7F;
	lastValue	= 0;
	value		= 0;

	for(int i=1; i<length+1; i++) {

		if(in[i]&0x80) {

			value = ((in[i]&0x7F) * 50) | PULSE_BIT;

			setData(space&PULSE_MASK);
			setData(value);

			space = 0;
		}
		else {
			space += ((in[i]&0x7F) * 50);
		}
	}

	if(length) {
		SetEvent(dataReadyEvent);
	}
}

void SendReceiveData::setData(lirc_t data) {

	//if(data&PULSE_BIT) printf("PULSE %i\n",data&PULSE_MASK);
	//else printf("SPACE %i\n",data&PULSE_MASK);

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

//======================================================================================
// sending stuff below
//======================================================================================



int SendReceiveData::send(ir_remote *remote, ir_ncode *code, int repeats) {

	//
	// Not supported for now
	//

	return 0;
}
