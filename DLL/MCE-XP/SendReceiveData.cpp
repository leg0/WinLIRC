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
#include <winlirc/PluginApi.h>
#include "../Common/Win32Helpers.h"
#include <tchar.h>

DWORD WINAPI MCEthread(void *recieveClass) {

	((SendReceiveData*)recieveClass)->threadProc();
	return 0;
}

SendReceiveData::SendReceiveData() {

	deviceHandle	= nullptr;
	exitEvent		= nullptr;

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

	deviceHandle = CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE,0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

	if(deviceHandle==INVALID_HANDLE_VALUE) {
		//printf("invalid device handle\n");
		return false;
	}

	threadHandle = CreateThread(nullptr,0,MCEthread,(void *)this,0,nullptr);

	if(!threadHandle) {
		return false;
	}

	return true;
}

void SendReceiveData::deinit() {

	KillThread(exitEvent,threadHandle);

	SAFE_CLOSE_HANDLE(exitEvent);
	SAFE_CLOSE_HANDLE(deviceHandle);
}

void SendReceiveData::threadProc() {

	//========================
	OVERLAPPED	overlappedRead;
	DWORD		result;
	UCHAR		buffer[256];
	DWORD		overflowCount;
	DWORD		bytesRead;
	//========================

	memset(&overlappedRead,0,sizeof(OVERLAPPED));

	overlappedRead.hEvent = CreateEvent(nullptr,FALSE,FALSE,nullptr);
	exitEvent = CreateEvent(nullptr,FALSE,FALSE,nullptr);

	HANDLE const events[] = { overlappedRead.hEvent, exitEvent };

	overflowCount = 0;

	while(true) {

overflowExit:

		ReadFile(deviceHandle, buffer+overflowCount, sizeof(buffer)-overflowCount, &bytesRead, &overlappedRead);

		result = WaitForMultipleObjects(2,events,FALSE,INFINITE);

		if(result==(WAIT_OBJECT_0)) 
		{
			//======================
			UCHAR	*bufferPointer;
			//======================

			GetOverlappedResult(deviceHandle, &overlappedRead, &bytesRead, FALSE);

			bufferPointer = buffer;

			while(bufferPointer < buffer+bytesRead+overflowCount) {

				if(*bufferPointer >= 0x81 && *bufferPointer <= 0x9E) {

					//===========
					UCHAR length;
					//===========

					length = *bufferPointer & 0x7F;

					if( length+1 > ((bytesRead + overflowCount) - (bufferPointer - buffer)) ) {
						overflowCount = (bytesRead + overflowCount) - (bufferPointer - buffer);
						memmove(buffer,bufferPointer,overflowCount);
						goto overflowExit;	//not read enough bytes to decode so wait for some more :p
					}

					decodeRaw(bufferPointer);

					bufferPointer += length + 1;
				}
				else if (*bufferPointer == 0x9F) {

					bufferPointer += 8;
					space += 100000;	//fake a space value, sigh :p
				}
				else {
					//skip unrecognised bytes
					bufferPointer += 1;
				}
			}

			overflowCount = 0;
		}

		else {
			CancelIo(deviceHandle);
			break;
		}
	}

	//printf("exited thread\n");
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

void SendReceiveData::stop() {

	UCHAR startPacket[] = {0x00, 0xFF, 0xAA};
}

void SendReceiveData::resume() {

	//================
	UCHAR *stopPacket;
	//================

	stopPacket = new UCHAR[194];

	stopPacket[0] = 0xFF;
	stopPacket[1] = 0xBB;

	for(int i=2; i<194; i++) {
		stopPacket[i] = 0xFF;
	}

	// send data

	delete [] stopPacket;
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
