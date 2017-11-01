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
#include "../Common/Send.h"
#include "../Common/DebugOutput.h"
#include "../Common/Win32Helpers.h"

DWORD WINAPI MCEthread(void *recieveClass) {

	((SendReceiveData*)recieveClass)->threadProc();
	return 0;
}

SendReceiveData::SendReceiveData() {

	deviceHandle		= nullptr;
	exitEvent			= nullptr;

	QueryPerformanceFrequency(&frequency);
}

bool SendReceiveData::init() {

	QueryPerformanceCounter(&lastTime);

	if(irDeviceList.get().empty()) {
		DPRINTF("No hardware.\n");
		return false;	//no hardware
	}

	deviceHandle = CreateFile(irDeviceList.get().begin()->c_str(), GENERIC_READ | GENERIC_WRITE,0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

	if(deviceHandle==INVALID_HANDLE_VALUE) {
		DPRINTF("invalid device handle.\n");
		return false;
	}

	transmitChannelMask = MCE_BLASTER_BOTH;

	resetHardware();

	if(!getCapabilities()) {
		DPRINTF("getCapabilities failed.\n");
		return false;
	}

	exitEvent = CreateEvent(nullptr,FALSE,FALSE,nullptr);

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

	//================================
	bool			interupted;
	bool			result;
	DWORD			bytesRead;
	ReceiveParams	*receiveParams;
	INT				receiveParamsSize;
	//================================

	if(!startReceive((int)receivePort,300)) {
		DPRINTF("Start failed.\n");
		return;
	}

	receiveParamsSize = sizeof(ReceiveParams) + (MCE_DEVICE_BUFFER_SIZE*4) + 8;
	receiveParams = (ReceiveParams *)calloc(1, receiveParamsSize);
	receiveParams->ByteCount = MCE_DEVICE_BUFFER_SIZE*4;

	while(true) {

		interupted	= false;
        result		= DeviceIo(IoCtrl_Receive, nullptr, 0, receiveParams, receiveParamsSize, bytesRead, INFINITE, false, interupted);

		if (result) {
			if ( bytesRead <= sizeof(ReceiveParams)) {
				continue;	// error of some sort try and recover
			}

			sendToDecoder((lirc_t *)((PUCHAR)receiveParams + sizeof(ReceiveParams)), (bytesRead - sizeof(ReceiveParams)) / 4);
		}
		else {
			resetHardware();
			startReceive((int)receivePort,300);
		}

		if(interupted) {
			break;
		}
	}

	free(receiveParams);

	stopReceive();

	DPRINTF("Thread exited.\n");
}

bool SendReceiveData::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==nullptr) evt=1;
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

bool SendReceiveData::DeviceIo( DWORD IoCtlCode, void * inBuffer, DWORD inBufferSize, void * outBuffer, DWORD outBufferSize, DWORD & bytesReturned, DWORD timeout )
{
    bool dummy;
    return DeviceIo( IoCtlCode, inBuffer, inBufferSize, outBuffer, outBufferSize, bytesReturned, timeout, true, dummy );
}

bool SendReceiveData::DeviceIo( DWORD IoCtlCode, void * inBuffer, DWORD inBufferSize, void * outBuffer, DWORD outBufferSize, DWORD & bytesReturned, DWORD timeout, bool dontInterrupt, bool & interupted )
{
	//=====================
	OVERLAPPED	overlapped;
	HANDLE		events[2];
	//=====================

	interupted = false;

	memset(&overlapped, 0, sizeof(overlapped));

	overlapped.hEvent = CreateEvent(0,false,false,0);
	
	events[0] = overlapped.hEvent;
	events[1] = exitEvent;

	if ( DeviceIoControl(deviceHandle, IoCtlCode, inBuffer, inBufferSize, outBuffer, outBufferSize, &bytesReturned, &overlapped) == FALSE )
	{
		if  (GetLastError() != ERROR_IO_PENDING )
		{
			CloseHandle(overlapped.hEvent);
			return false;
		}
	}

	int eventCnt = 2;
	if ( dontInterrupt ) 
		eventCnt = 1;

	DWORD r = WaitForMultipleObjects( eventCnt, events, FALSE, timeout );
	CloseHandle(overlapped.hEvent);
	switch ( r )
	{
	case WAIT_TIMEOUT:
		DPRINTF("DeviceIo timed out\n");
		CancelIo(deviceHandle);
		return false;
	case WAIT_FAILED:
		DPRINTF("DeviceIo wait failed %i\n",GetLastError());
		CancelIo(deviceHandle);
		return false;
	case WAIT_OBJECT_0 + 1: // the event object!
		CancelIo(deviceHandle);
		DPRINTF("DeviceIo exit event was set\n");
		ResetEvent(exitEvent);
		interupted = true;
		return false;
	}

	if (GetOverlappedResult(deviceHandle, (LPOVERLAPPED)&overlapped, &bytesReturned, FALSE) == FALSE )
	{
		DPRINTF("DeviceIo OverlappedResult error %i\n", GetLastError());
		return false;
	}

	return true;
}

bool SendReceiveData::getCapabilities() {

	//==================
	DWORD bytesReturned;
	//==================

	if (!DeviceIo(IoCtrl_GetDetails, nullptr, 0, &mceDeviceCapabilities, sizeof(MCEDeviceCapabilities), bytesReturned, 5000)) {
		return false;
    }
	
    if (bytesReturned < sizeof(MCEDeviceCapabilities)) {
        return false;
    }

    receivePort	= FirstLowBit((int)mceDeviceCapabilities.LearningMask);

	DPRINTF("ProtocolVersion %i\n",mceDeviceCapabilities.ProtocolVersion);
	DPRINTF("NumTransmitPorts %i\n",mceDeviceCapabilities.NumTransmitPorts);
	DPRINTF("NumReceivePorts %i\n",mceDeviceCapabilities.NumReceivePorts);
	DPRINTF("LearningMask %i\n",mceDeviceCapabilities.LearningMask);
	DPRINTF("DetailsFlags %i\n",mceDeviceCapabilities.DetailsFlags);
	DPRINTF("receive port %i\n",receivePort);

	return true;
}

bool SendReceiveData::resetHardware() {

	//==============
	DWORD bytesRead;
	//==============

    if (!DeviceIo(IoCtrl_Reset, nullptr, 0, nullptr, 0, bytesRead, 5000)) {
		return false;
    }

	return true;
}

bool SendReceiveData::startReceive(int portToUse, int timeout)
{
	//============================
    DWORD				bytesRead;
    StartReceiveParams	parms;
	//============================

    parms.Receiver	= portToUse;
    parms.Timeout	= timeout;
        
    if (!DeviceIo(IoCtrl_StartReceive, &parms, sizeof(parms), nullptr, 0, bytesRead, 5000)) {
		return false;
    }
	
	return true;
}

bool SendReceiveData::stopReceive() {

	//==============
    DWORD bytesRead;
	//==============
        
    if (!DeviceIo(IoCtrl_StopReceive, nullptr, 0, nullptr, 0, bytesRead, 5000)) {
		return false;
    }
    
	return true;
}

void SendReceiveData::sendToDecoder(lirc_t *pData, int len) {

	//===============
	lirc_t value;
	lirc_t totalTime;
	//===============

	//
	// get total length in time
	//

	totalTime = 0;

	for(int i = 0; i < len; i++) {

		if (pData[i] < 0) {
			value = pData[i] * -1;
		}
		else {
			value = pData[i];
		}

		totalTime += value;
	}

	{
		//====================
		lirc_t timeDifference;
		//====================

		QueryPerformanceCounter(&time);

		timeDifference	= (lirc_t)(((time.QuadPart - lastTime.QuadPart)*1000000) / frequency.QuadPart);
		lastTime		= time;

		//
		// Time out value is 30,000
		//
		timeDifference = timeDifference - totalTime + 30000;

		//
		// sanity checking
		//
		if(timeDifference<0 || timeDifference>PULSE_MASK) {
			timeDifference = PULSE_MASK;
		}

		setData(timeDifference);
	}

    for (int i = 0; i < len-1; i++) {

		if (pData[i] < 0) {
			value = pData[i] * -1;				//space
		}
		else {
			value = pData[i] | PULSE_BIT;		//pulse
		}		

		setData(value);
    }

	SetEvent(dataReadyEvent);
}




//======================================================================================
// sending stuff below
//======================================================================================

bool SendReceiveData::getAvailableBlasters() {

	//===================
	DWORD bytesReturned;
	//===================

	if ( ! DeviceIo(IoCtrl_GetBlasters, nullptr, 0, &availableBlasters, sizeof(AvailableBlasters),bytesReturned, 5000) ) {
		availableBlasters.Blasters = 0;
		return false;
	}

	DPRINTF("Available blasters %i %i\n",availableBlasters.Blasters,bytesReturned);

	return true;     
}


int SendReceiveData::send(ir_remote *remote, ir_ncode *code, int repeats) {

	//===================
	int blasterPort;
	int carrierFrequency;
	//===================

	blasterPort = calcBlasterPort();

	if(!blasterPort) {
		return 0;			// not set any blaster ports
	}

	if(remote->freq==0) {
		carrierFrequency = 38000;
	}
	else {
		carrierFrequency = remote->freq;
	}

	if (init_send(remote, code, repeats)) {

		//====================
		int		*mceData;
		bool	space;
		BOOL	success;
		//====================

		space = false;	//always start sending a pulse

		//
		// stop recieving thread - don't necessarily need this
		//
		KillThread(exitEvent,threadHandle);

		auto const length		= get_send_buffer_length();
		auto const signals		= get_send_buffer_data();

		mceData		= new int[length];

		for(int i=0; i<length; i++) {

			mceData[i] = signals[i];
			mceData[i] = ((mceData[i]+25)/50) * 50;	//round to nearest 50 (is this needed for blasting ?)
			
			if(space) {
				mceData[i] = mceData[i] * -1;
			}

			space = !space;
		}

		success = transmit(mceData,length,blasterPort,1000000/carrierFrequency);

		delete [] mceData;

		//
		// Start receiving again
		//
		threadHandle = CreateThread(nullptr,0,MCEthread,(void *)this,0,nullptr);

		return success;
	}

	return 0;
}

BOOL SendReceiveData::transmit(int *data, size_t dataLen, int transmitMask, int period) {

	//==================
    DWORD			bytesReturned;
    TransmitParams	params = {transmitMask,period,0,0};
	UCHAR			*formattedData;
	TransmitChunk	*transmitChunk;
	INT				*offsettedData;
	//==================

	formattedData = new UCHAR[sizeof(TransmitChunk) + (sizeof(INT) * dataLen) + 8];

	transmitChunk = (TransmitChunk*)formattedData;

	transmitChunk->byteCount			= dataLen * sizeof(INT);
	transmitChunk->offsetToNextChunk	= 0;
	transmitChunk->repeatCount			= 1;	//1 ?

	offsettedData = (INT*)(formattedData + sizeof(TransmitChunk));

	for(UINT i=0; i<dataLen; i++) {
		offsettedData[i] = data[i];
	}

    if (!DeviceIo(IoCtrl_Transmit, &params, sizeof(TransmitParams), formattedData, sizeof(TransmitChunk) + (sizeof(INT) * dataLen) + 8, bytesReturned, 5000 )) {
		delete [] formattedData;
		return false;
    }

	delete [] formattedData;
    return true;
}

int SendReceiveData::calcBlasterPort() {
	
	//================
	int tempPort;
	//================

	getAvailableBlasters();

	tempPort = transmitChannelMask;

	switch(tempPort) {

	case MCE_BLASTER_BOTH:
		tempPort = (int)availableBlasters.Blasters;
		break;
	case MCE_BLASTER_1:
		tempPort = GetHighBit((int)availableBlasters.Blasters,(int)mceDeviceCapabilities.NumTransmitPorts-1);
		break;
	case MCE_BLASTER_2:
		tempPort = GetHighBit((int)availableBlasters.Blasters,(int)mceDeviceCapabilities.NumTransmitPorts);
	break;
		default:
		return 0;
	}

	return tempPort;
}

void SendReceiveData::setTransmitters(unsigned int channelMask) {

	transmitChannelMask = channelMask;
}