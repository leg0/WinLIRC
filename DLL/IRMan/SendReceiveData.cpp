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
#include "../Common/DebugOutput.h"
#include "../Common/Linux.h"

DWORD WINAPI IRMan(void *recieveClass) {

	((SendReceiveData*)recieveClass)->threadProc();
	return 0;
}

SendReceiveData::SendReceiveData() {

	m_threadHandle		= NULL;
	m_exitEvent			= NULL;
	m_overlappedEvent	= NULL;

	memset(&m_overlapped,0,sizeof(OVERLAPPED));
}

bool SendReceiveData::init() {

	//===================
	TCHAR comPortName[32];
	char tempBuffer[32];
	//===================

	irCode = 0;
	gettimeofday(&end,NULL);	// initialise

	_sntprintf(comPortName,_countof(comPortName),_T("\\\\.\\COM%i"),settings.getComPort());

	TPRINTF(_T("Opening port %s\n"),comPortName);

	if(CSerial::CheckPort(comPortName)!=CSerial::EPortAvailable) {
		DPRINTF("Port unavailable\n"); 	
		return false;
	}

	//open serial port
	if(m_serial.Open(comPortName,0,0,true)!=ERROR_SUCCESS) {
		DPRINTF("Opening port failed\n");
		return false;
	}

	if(m_serial.Setup(CSerial::EBaud9600,CSerial::EData8,CSerial::EParNone,CSerial::EStop1)!=ERROR_SUCCESS) {
		DPRINTF("Setting up failed\n");
		return false;
	}

	if(m_serial.SetupHandshaking(CSerial::ERTSDTR)!=ERROR_SUCCESS) {
		DPRINTF("Setting up handshaking failed\n");
		return false;
	}

	if(m_serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking)!=ERROR_SUCCESS) {
		DPRINTF("Setting up read timeouts failed\n");
		return false;
	}

	// clear read buffer if there is any data in there

	DWORD bytesRead;
	while(m_serial.Read(tempBuffer,1,&bytesRead)==ERROR_SUCCESS) {
		if(bytesRead==0) break;
	}

	// Send IR to the device and await response

	tempBuffer[0] = 'I';
	tempBuffer[1] = 'R';

	m_serial.Write(tempBuffer,1);
	Sleep(250);
	m_serial.Write(tempBuffer+1,1);
	Sleep(250);

	m_serial.Read(tempBuffer,sizeof(tempBuffer));

	if(strncmp(tempBuffer,"OK",2)) {
		DPRINTF("OK not returned from the device\n");
		return false;
	}

	m_serial.SetMask(CSerial::EEventRecv);
	
	m_exitEvent			= CreateEvent(NULL,FALSE,FALSE,NULL);
	m_overlappedEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);
	m_overlapped.hEvent	= m_overlappedEvent;
	m_threadHandle		= CreateThread(NULL,0,IRMan,(void *)this,0,NULL);

	if(m_threadHandle) {
		return true;
	}

	return false;
}

void SendReceiveData::deinit() {

	killThread();

	if(m_exitEvent) {
		CloseHandle(m_exitEvent);
		m_exitEvent = NULL;
	}

	if(m_overlappedEvent) {
		CloseHandle(m_overlappedEvent);
		m_overlappedEvent = NULL;
	}
}


void SendReceiveData::threadProc() {

	receiveLoop();
}

void SendReceiveData::killThread() {

	//
	// need to kill thread here
	//

	SetEvent(m_exitEvent);

	if(m_threadHandle!=NULL) {

		//===========
		DWORD result;
		//===========

		result = 0;

		if(GetExitCodeThread(m_threadHandle,&result)==0) 
		{
			CloseHandle(m_threadHandle);
			m_threadHandle = NULL;
			return;
		}

		if(result==STILL_ACTIVE)
		{
			WaitForSingleObject(m_threadHandle,INFINITE);
		}

		CloseHandle(m_threadHandle);
		m_threadHandle = NULL;
	}
}

bool SendReceiveData::waitTillDataIsReady(int maxUSecs) {

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
			return false;
		}
	}

	return true;
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

	//==============
	HANDLE	wait[2];
	DWORD	result;
	//==============

	wait[0] = m_overlappedEvent;
	wait[1] = m_exitEvent;

	while(1)
	{
		//=====================
		CSerial::EEvent eEvent;
		//=====================

		m_serial.WaitEvent(&m_overlapped);

		result = WaitForMultipleObjects(_countof(wait),wait,FALSE,INFINITE);

		if(result==WAIT_OBJECT_0) {

			//==================
			DWORD	dwBytesRead;
			char	buffer[256];
			//==================

			eEvent = m_serial.GetEventType();

			if (!(eEvent & CSerial::EEventRecv)) { DPRINTF("wrong event\n"); continue; }

			gettimeofday(&start,NULL);

			while(1) {

				if(m_serial.Read(buffer,sizeof(buffer),&dwBytesRead)!=ERROR_SUCCESS) {
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

				DPRINTF("ir code %I64d\n",irCode);

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

	DPRINTF("Thread exited\n");

	m_serial.Close();
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

