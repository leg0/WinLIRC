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
#include <tchar.h>
#include "SendReceiveData.h"
#include "Globals.h"
#include "../Common/DebugOutput.h"
#include "../Common/Win32Helpers.h"
#include <iterator>

DWORD WINAPI IRMan(void *recieveClass) {

	((SendReceiveData*)recieveClass)->receiveLoop();
	return 0;
}

SendReceiveData::SendReceiveData() {

	m_threadHandle		= nullptr;
	m_exitEvent			= nullptr;
	m_overlappedEvent	= nullptr;

	memset(&m_overlapped,0,sizeof(OVERLAPPED));
}

bool SendReceiveData::init() {

	//===================
	TCHAR comPortName[32];
	char tempBuffer[32];
	//===================

	m_count	= 0;
	irCode	= 0;

	end = std::chrono::steady_clock::now();

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

	// Lines for RTS and DTR should now be live. Give the device a little time to boot up.
	Sleep(250);

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
		m_serial.Close();
		return false;
	}

	m_serial.SetMask(CSerial::EEventRecv);
	
	m_exitEvent			= CreateEvent(nullptr,FALSE,FALSE,nullptr);
	m_overlappedEvent	= CreateEvent(nullptr,FALSE,FALSE,nullptr);
	m_overlapped.hEvent	= m_overlappedEvent;
	m_threadHandle		= CreateThread(nullptr,0,IRMan,(void *)this,0,nullptr);

	if(m_threadHandle) {
		return true;
	}

	return false;
}

void SendReceiveData::deinit() {

	KillThread(m_exitEvent,m_threadHandle);

	SAFE_CLOSE_HANDLE(m_exitEvent);
	SAFE_CLOSE_HANDLE(m_overlappedEvent);
}

bool SendReceiveData::waitTillDataIsReady(std::chrono::microseconds maxUSecs) {

	HANDLE const events[2]={dataReadyEvent,threadExitEvent};
	DWORD const evt = (threadExitEvent==nullptr) ? 1 : 2;

	if(!dataReady())
	{
		ResetEvent(dataReadyEvent);
		using namespace std::chrono;
		DWORD const dwTimeout = maxUSecs > 0us
			? duration_cast<milliseconds>(maxUSecs + 500us).count()
			: INFINITE;
		DWORD const res = ::WaitForMultipleObjects(evt, events, false, dwTimeout);
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

	HANDLE const wait[2] = { m_overlappedEvent, m_exitEvent };

	while(1)
	{
		m_serial.WaitEvent(&m_overlapped);

		auto const result = WaitForMultipleObjects(std::size(wait),wait,FALSE,INFINITE);

		if(result==WAIT_OBJECT_0) {

			auto const eEvent = m_serial.GetEventType();

			if (!(eEvent & CSerial::EEventRecv)) { DPRINTF("wrong event\n"); continue; }

			if(m_count==0) {
				start = std::chrono::steady_clock::now();
			}

			DWORD dwBytesRead;
			char buffer[6];
			if(m_serial.Read(buffer+m_count,sizeof(buffer)-m_count,&dwBytesRead)!=ERROR_SUCCESS) {
				break;					// read error
			}

			m_count += dwBytesRead;

			if(m_count==6) {

				EnterCriticalSection(&criticalSection);

				last = end;
				irCode = 0xffff;

				for(DWORD i=0; i<6; i++) {
					irCode = irCode<<8;
					irCode = irCode|(ir_code) (unsigned char) buffer[i];					
				}

				m_count = 0;

				LeaveCriticalSection(&criticalSection);

				end = std::chrono::steady_clock::now();

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

