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
#include "../Common/Send.h"

DWORD WINAPI IRToy(void *recieveClass) {

	((SendReceiveData*)recieveClass)->threadProc();
	return 0;
}

SendReceiveData::SendReceiveData() {

	bufferStart		= 0;
	bufferEnd		= 0;
	threadHandle	= NULL;
	exitEvent		= NULL;
	overlappedEvent	= NULL;

	memset(&overlapped,0,sizeof(OVERLAPPED));
}

bool SendReceiveData::init() {

	//===================
	TCHAR comPortName[10];
	char tempBuffer[32];
	//===================

	_sntprintf(comPortName,_countof(comPortName),_T("\\\\.\\COM%i"),settings.getComPort());

	if(CSerial::CheckPort(comPortName)!=CSerial::EPortAvailable) return false;

	//printf("INIT\n");

	//
	//open serial port and set stuff up
	//
	if(serial.Open(comPortName,0,0,true)!=ERROR_SUCCESS) return false;;
	if(serial.Setup(CSerial::EBaud115200,CSerial::EData8,CSerial::EParNone,CSerial::EStop1)!=ERROR_SUCCESS) return false;
	if(serial.SetupHandshaking(CSerial::EHandshakeOff)!=ERROR_SUCCESS) return false;	//change this to none?
	if(serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking)!=ERROR_SUCCESS) return false;

	for(int i=0;i<5; i++) {
		tempBuffer[i] = '\0';
	}
	if(serial.Write(tempBuffer,5)!=ERROR_SUCCESS) return false;
	Sleep(50);

	//
	// sample mode
	//
	tempBuffer[0] = 's';
	if(serial.Write(tempBuffer,1)!=ERROR_SUCCESS) return false;
	Sleep(100);
	serial.Read(tempBuffer,sizeof(tempBuffer));

	//
	// transmit hand shake
	//
	tempBuffer[0] = 0x26;
	if(serial.Write(tempBuffer,1)!=ERROR_SUCCESS) return false;
	Sleep(20);

	//
	// Notify on complete 
	//
	tempBuffer[0] = 0x25;
	if(serial.Write(tempBuffer,1)!=ERROR_SUCCESS) return false;
	Sleep(20);

	if(serial.SetupHandshaking(CSerial::EHandshakeHardware)!=ERROR_SUCCESS) return false;

	exitEvent			= CreateEvent(NULL,FALSE,FALSE,NULL);
	overlappedEvent		= CreateEvent(NULL,FALSE,FALSE,NULL);
	overlapped.hEvent	= overlappedEvent;
	threadHandle		= CreateThread(NULL,0,IRToy,(void *)this,0,NULL);

	if(threadHandle) {
		return true;
	}

	return false;
}

void SendReceiveData::deinit() {

	killThread();

	serial.Close();

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

void SendReceiveData::setData(lirc_t data) {

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool SendReceiveData::dataReady() {

	if(bufferStart==bufferEnd) {
		return false;
	}

	return true;
}

bool SendReceiveData::getData(lirc_t *out) {

	if(!dataReady()) {
		return false;
	}

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

void SendReceiveData::receiveLoop() {

	//=================
	HANDLE	wait[2];
	DWORD	result;
	bool	highByte;
	bool	pulse;
	lirc_t	value;
	bool	firstSpace;
	char	tempBuffer[1024];
	//=================

	wait[0] = overlappedEvent;
	wait[1] = exitEvent;

	highByte	= true;
	pulse		= true;
	value		= 0;
	firstSpace	= false;

	//
	// read any random data waiting - we don't care if this fails
	//
	serial.Read(tempBuffer,sizeof(tempBuffer));

	serial.SetMask(CSerial::EEventRecv);	//register events we want to handle

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

			if (!(eEvent & CSerial::EEventRecv)) { /*printf("wrong event\n");*/ continue; }

			while(1) {

				if(serial.Read(buffer,sizeof(buffer),&dwBytesRead)!=ERROR_SUCCESS) {
					break;					// read error
				}

				if(dwBytesRead==0) break;

				for(DWORD i=0; i<dwBytesRead;i++) {
				
					if(highByte) {
						value = (buffer[i] & 0xFF) << 8;
						highByte = false;
					}
					else {

						value		= value + (buffer[i] & 0xFF);
						highByte	= true;			//next byte is 'high

						if(value==0xFFFF) {			//resync
							highByte	= true;
							pulse		= true;
							firstSpace	= false;
							break;
						}

						value = (int)((float)value * 21.3333f);

						if(pulse) {
							value |= PULSE_BIT;
							pulse = false;
						}
						else {
							pulse = true;			//next value will be a pulse;
						}

						//
						// bit of a hack
						// add space after a time-out value has been received
						// we append it to the next pulse to prevent problems
						//
						if(!firstSpace) {
							setData(PULSE_MASK);
							firstSpace = true;
						}

						setData(value);
						SetEvent(dataReadyEvent);
					}
				}
			}

		}
		else if(result==WAIT_OBJECT_0+1) {
			break;	//exit thread
		}
		else {
			break;	//unknown error/event
		}
	}
}

//======================================================================================
// sending stuff below
//======================================================================================

UCHAR SendReceiveData::calcPR2(int frequency) {

	//======================
	double	oscillatorPeriod;
	double	pwmPeriod;
	int		pr2;
	//======================

	oscillatorPeriod	= 1.0/48000000.0;
	pwmPeriod			= 1.0/(double)frequency;

	pr2 = (int)((pwmPeriod/(16 * oscillatorPeriod))-0.5);

	if(pr2==0 || pr2>255) {
		return 0;	//failed to match a value
	}

	return (UCHAR)pr2;
}

int SendReceiveData::send(ir_remote *remote, ir_ncode *code, int repeats) {

	if (init_send(remote, code, repeats)) {

		//====================
		int		length;
		lirc_t	*signals;
		USHORT	*irToySignals;
		UCHAR	temp[3];
		BOOL	success;
		//====================

		success = FALSE;

		//
		// stop recieving thread - don't necessarily need this
		//
		killThread();

		serial.SetMask(CSerial::EEventNone);
		serial.SetupReadTimeouts(CSerial::EReadTimeoutBlocking);

		if(remote->freq) {

			//======
			int pr2;
			//======

			pr2 = calcPR2(remote->freq);

			if(pr2) {

				temp[0] = 0x06;	//set frequency 
				temp[1] = pr2;	//PR2 value
				temp[2] = 0;	//don't care value

				serial.Write(temp,3);
				Sleep(50);
			}
		}

		length		= send_buffer.wptr+1;
		signals		= send_buffer.data;
		temp[0]		= 0x03;	// transmit mode
		irToySignals= (USHORT*)malloc(sizeof(USHORT) * (length)); // add 1 for 0xFFFF terminator
		
		serial.Write(temp,1);	// set transmit mode

		for(int i=0; i<length-1; i++) {

			irToySignals[i] = (USHORT)((signals[i] & PULSE_MASK) / 21.33);

			//
			// swap bytes
			//
			temp[0]			= ((irToySignals[i]>>8) & 0x00FF);
			irToySignals[i]	= (irToySignals[i]<<8) & 0xFF00;
			irToySignals[i] = irToySignals[i] + temp[0];
		}

		//
		// if we end in a space remove it
		//
		if(length%2==1) {
			length--;
		}

		irToySignals[length-1] = 0xFFFF;	//terminate sending !

		//
		// split data up into packets for IR Toy, so as to not cause an overflow
		//

		{
			//=====================
			UCHAR	packetLength;
			DWORD	bytesRead;
			USHORT	*irToySignalsP;
			//=====================

			irToySignalsP = irToySignals;

			for(int i=0; i<length; ) {

				serial.Read(&packetLength,1,&bytesRead);

				if(length-i<packetLength/2) {
					packetLength = (length-i)*2;	// in bytes
				}

				serial.Write(irToySignalsP,packetLength);
				irToySignalsP += packetLength/2;
				i += packetLength/2;
			}

			serial.Read(&packetLength,1,&bytesRead);
		}

		//
		// get notify on transmit complete
		//

		{
			//=====================
			UCHAR transmitComplete;
			DWORD bytesRead;
			//=====================

			serial.Read(&transmitComplete,1,&bytesRead);

			if(bytesRead) {

				if(transmitComplete=='C') {
					success = TRUE;
				}
				else if(transmitComplete=='F') {
					success = FALSE;
				}
				else {
					success = FALSE;
				}
			}
		}

		free(irToySignals);

		//
		// restore non blocking mode
		//
		serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);

		//
		// Start receiving again
		//
		threadHandle = CreateThread(NULL,0,IRToy,(void *)this,0,NULL);

		return success;
	}

	return 0;
}

