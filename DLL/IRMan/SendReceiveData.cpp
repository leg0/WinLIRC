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
#include <tchar.h>

DWORD WINAPI IRMan(void *recieveClass) {

	((SendReceiveData*)recieveClass)->threadProc();
	return 0;
}

SendReceiveData::SendReceiveData() {

	threadHandle	= NULL;
	exitEvent		= NULL;
	overlappedEvent	= NULL;

	memset(&overlapped,0,sizeof(OVERLAPPED));
}

bool SendReceiveData::init() {

	//===================
	TCHAR comPortName[8];
	char tempBuffer[32];
	//===================

	irCode = 0;
	gettimeofday(&end,NULL);	// initialise

	_sntprintf(comPortName,_countof(comPortName),_T("COM%i"),settings.getComPort());

	//_tprintf(_T("com port name %s\n"),comPortName);

	if(CSerial::CheckPort(comPortName)!=CSerial::EPortAvailable) return false;

	//printf("INIT\n");

	//
	//open serial port
	//
	if(serial.Open(comPortName,0,0,true)!=ERROR_SUCCESS) return false;;
	if(serial.Setup(CSerial::EBaud9600,CSerial::EData8,CSerial::EParNone,CSerial::EStop1)!=ERROR_SUCCESS) return false;
	if(serial.SetupHandshaking(CSerial::ERTSDTR)!=ERROR_SUCCESS) return false;	//change this to none?
	if(serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking)!=ERROR_SUCCESS) return false;
	tempBuffer[0] = 'I';
	if(serial.Write(tempBuffer,1)!=ERROR_SUCCESS) return false;
	Sleep(250);
	tempBuffer[0] = 'R';
	if(serial.Write(tempBuffer,1)!=ERROR_SUCCESS) return false;
	Sleep(250);
	serial.Read(tempBuffer,sizeof(tempBuffer));

	tempBuffer[2] = '\0';	// null terminate string

	if(strcmp(tempBuffer,"OK")) return false;	// okay not returned

	serial.SetMask(CSerial::EEventRecv);
	
	exitEvent			= CreateEvent(NULL,FALSE,FALSE,NULL);
	overlappedEvent		= CreateEvent(NULL,FALSE,FALSE,NULL);
	overlapped.hEvent	= overlappedEvent;
	threadHandle		= CreateThread(NULL,0,IRMan,(void *)this,0,NULL);

	if(threadHandle) {
		return true;
	}

	return false;
}

void SendReceiveData::deinit() {

	killThread();

	if(exitEvent) {
		CloseHandle(exitEvent);
		exitEvent = NULL;
	}

	if(overlappedEvent) {
		CloseHandle(overlappedEvent);
		overlappedEvent = NULL;
	}

}


void SendReceiveData::threadProc() {

	receiveLoop();
}

void SendReceiveData::killThread() {

	//
	// need to kill thread here
	//

	SetEvent(exitEvent);

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


int SendReceiveData::dataReady() {

	//=========
	int result;
	//=========

	result = WaitForSingleObject(dataReadyEvent,0);

	if(result==WAIT_OBJECT_0) {
		return 1;
	}
	
	return 0;
}

void SendReceiveData::receiveLoop() {

	//=================
	HANDLE	wait[2];
	DWORD	result;
	//=================

	wait[0] = overlappedEvent;
	wait[1] = exitEvent;


	//printf("entering receive loop !!!!!!!!!\n");

	while(1)
	{
		//=====================
		CSerial::EEvent eEvent;
		//=====================

		serial.WaitEvent(&overlapped);

		result = WaitForMultipleObjects(sizeof(wait)/sizeof(*wait),wait,FALSE,INFINITE);

		if(result==WAIT_OBJECT_0) {

			//==================
			DWORD	dwBytesRead;
			char	buffer[256];
			//==================

			eEvent = serial.GetEventType();

			if (!(eEvent & CSerial::EEventRecv)) { printf("wrong event\n"); continue; }

			gettimeofday(&start,NULL);

			while(1) {

				if(serial.Read(buffer,sizeof(buffer),&dwBytesRead)!=ERROR_SUCCESS) {
					break;					// read error
				}

				if(dwBytesRead<=0) break;	//finished reading

				if(dwBytesRead!=6) break;

				//
				// assume we will read 6 bytes at a time if not we fail :p
				//

				EnterCriticalSection(&criticalSection);

				last = end;
				irCode = 0xffff;

				for(DWORD i=0; i<dwBytesRead;i++) {
					irCode = irCode<<8;
					irCode = irCode|(ir_code) (unsigned char) buffer[i];					
				}

				//printf("ir code %I64d\n",irCode);

				LeaveCriticalSection(&criticalSection);

				gettimeofday(&end,NULL);

				SetEvent(dataReadyEvent);
			}

		}
		else if(result==WAIT_OBJECT_0+1) {
			break;	//exit thread
		}
		else {
			break;	//unknown error/event
		}
	}

	//printf("thread exited\n");

	serial.Close();
}

//======================================================================================
// sending stuff below
//======================================================================================

int SendReceiveData::send(ir_remote *remote, ir_ncode *code, int repeats) {

	//
	// not supported for now, if we do support this we'll need to halt receiving
	//

	return 0;
}

