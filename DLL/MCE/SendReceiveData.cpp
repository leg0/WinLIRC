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
#include "../Common/Send.h"

DWORD WINAPI MCEthread(void *recieveClass) {

	((SendReceiveData*)recieveClass)->threadProc();
	return 0;
}

SendReceiveData::SendReceiveData() {

	deviceHandle		= NULL;
	exitEvent			= NULL;

	QueryPerformanceFrequency(&frequency);
}

bool SendReceiveData::init() {

	QueryPerformanceCounter(&lastTime);

	if(irDeviceList.get().empty()) return false;	//no hardware

	deviceHandle = CreateFile(irDeviceList.get().begin()->c_str(), GENERIC_READ | GENERIC_WRITE,0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if(deviceHandle==INVALID_HANDLE_VALUE) {
		//printf("invalid device handle\n");
		return false;
	}

	transmitChannelMask = MCE_BLASTER_BOTH;

	resetHardware();

	if(!getCapabilities()) {
		//printf("getCapabilities fail\n");
		return false;
	}

	exitEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

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

	//================================
	bool			interupted;
	bool			result;
	DWORD			bytesRead;
	ReceiveParams	*receiveParams;
	INT				receiveParamsSize;
	//================================

	if(!startReceive((int)receivePort,30)) {
		//printf("start receive fail\n");
		return;
	}

	receiveParamsSize = sizeof(ReceiveParams) + (MCE_DEVICE_BUFFER_SIZE*4) + 8;
	receiveParams = (ReceiveParams *)calloc(1, receiveParamsSize);
	receiveParams->ByteCount = MCE_DEVICE_BUFFER_SIZE*4;

	while(true) {

		//printf("in thread loop\n");

		interupted	= false;
        result		= DeviceIo(IoCtrl_Receive, NULL, 0, receiveParams, receiveParamsSize, bytesRead, INFINITE, false, interupted);

		if (result) {
			if ( bytesRead <= sizeof(ReceiveParams)) {
				//printf("failed somehow \n");
				continue;	// error of some sort try and recover
			}

			sendToDecoder((lirc_t *)((PUCHAR)receiveParams + sizeof(ReceiveParams)), (bytesRead - sizeof(ReceiveParams)) / 4);
			//printf("decode !\n");
		}
		else {
			//printf("failed somehow\n");
			//break;
		}
		if(interupted) {
			break;
		}
	}

	free(receiveParams);

	stopReceive();

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
		//m_Logger->log( ILogger::L_ERROR, "DeviceIo timed out.");
		CancelIo(deviceHandle);
		return false;
	case WAIT_FAILED:
		//m_Logger->log( ILogger::L_ERROR, "DeviceIo wait failed", GetLastError());
		CancelIo(deviceHandle);
		return false;
	case WAIT_OBJECT_0 + 1: // the event object!
		CancelIo(deviceHandle);
		//m_Logger->log( ILogger::L_INFO, "DeviceIo m_Event was set", GetLastError());
		ResetEvent(exitEvent);
		interupted = true;
		return false;
	}

	if (GetOverlappedResult(deviceHandle, (LPOVERLAPPED)&overlapped, &bytesReturned, FALSE) == FALSE )
	{
		//m_Logger->log( ILogger::L_ERROR, "DeviceIo OverlappedResult Error", GetLastError());
		return false;
	}

	return true;
}

bool SendReceiveData::getCapabilities() {

	//==================
	DWORD bytesReturned;
	//==================

	if (!DeviceIo(IoCtrl_GetDetails, NULL, 0, &mceDeviceCapabilities, sizeof(MCEDeviceCapabilities), bytesReturned, 5000)) {
		return false;
    }
	
    if (bytesReturned < sizeof(MCEDeviceCapabilities)) {
        return false;
    }

    receivePort	= FirstLowBit((int)mceDeviceCapabilities.LearningMask);

	//printf("ProtocolVersion %i\n",mceDeviceCapabilities.ProtocolVersion);
	//printf("NumTransmitPorts %i\n",mceDeviceCapabilities.NumTransmitPorts);
	//printf("NumReceivePorts %i\n",mceDeviceCapabilities.NumReceivePorts);
	//printf("LearningMask %i\n",mceDeviceCapabilities.LearningMask);
	//printf("DetailsFlags %i\n",mceDeviceCapabilities.DetailsFlags);

	//printf("receive port %i\n",receivePort);

	return true;
}

bool SendReceiveData::resetHardware() {

	//==============
	DWORD bytesRead;
	//==============

    if (!DeviceIo(IoCtrl_Reset, NULL, NULL, NULL, 0, bytesRead, 5000)) {
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
        
    if (!DeviceIo(IoCtrl_StartReceive, &parms, sizeof(parms), NULL, 0, bytesRead, 5000)) {
		return false;
    }
	
	return true;
}

bool SendReceiveData::stopReceive() {

	//==============
    DWORD bytesRead;
	//==============
        
    if (!DeviceIo(IoCtrl_StopReceive, NULL, 0, NULL, 0, bytesRead, 5000)) {
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

		//printf("added value %i\n",timeDifference);
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

	if ( ! DeviceIo(IoCtrl_GetBlasters, NULL, 0, &availableBlasters, sizeof(AvailableBlasters),bytesReturned, 5000) ) {
		availableBlasters.Blasters = 0;
		return false;
	}

	//printf("available blasters %i %i\n",availableBlasters.Blasters,bytesReturned);

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
		int		length;
		lirc_t	*signals;
		int		*mceData;
		bool	space;
		BOOL	success;
		//====================

		space = false;	//always start sending a pulse

		//
		// stop recieving thread - don't necessarily need this
		//
		killThread();

		length		= send_buffer.wptr;
		signals		= send_buffer.data;

		mceData		= new int[length];

		for(int i=0; i<length; i++) {

			mceData[i] = signals[i];
			mceData[i] = ((mceData[i]+25)/50) * 50;	//round to nearest 50 (is this needed for blasting ?)
			
			if(space) {
				mceData[i] = mceData[i] * -1;
			}

			space = !space;

			//printf("mceData[%i] %i\n",i,mceData[i]);
		}

		success = transmit(mceData,length,blasterPort,1000000/carrierFrequency);

		//printf("transmit success %i\n",success);

		delete [] mceData;

		//
		// Start receiving again
		//
		threadHandle = CreateThread(NULL,0,MCEthread,(void *)this,0,NULL);

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

	//printf("transmit port number %i\n",tempPort);

	return tempPort;
}

void SendReceiveData::setTransmitters(unsigned int channelMask) {

	transmitChannelMask = channelMask;
}