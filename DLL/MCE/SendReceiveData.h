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

#ifndef RECEIVEDATA_H
#define RECEIVEDATA_H

#include "../Common/LIRCDefines.h"
#include "MCEDefines.h"
#include "IrDeviceList.h"

class SendReceiveData
{
public:
	SendReceiveData();

	bool	init();
	void	deinit();

	bool	getData(lirc_t *out);
	bool	dataReady();
	bool	waitTillDataIsReady(std::chrono::microseconds maxUSecs);
	void	threadProc();
	int		send(struct ir_remote *remote, struct ir_ncode *code, int repeats);
	void	setTransmitters(unsigned int channelMask);
	
private:

	void	setData(lirc_t data);

	bool	DeviceIo( DWORD IoCtlCode, void * inBuffer, DWORD inBufferSize, void * outBuffer, DWORD outBufferSize, DWORD & bytesReturned, DWORD timeout );
	bool	DeviceIo( DWORD IoCtlCode, void * inBuffer, DWORD inBufferSize, void * outBuffer, DWORD outBufferSize, DWORD & bytesReturned, DWORD timeout, bool dontInterrupt, bool & interupted );

	bool	getCapabilities	();
	bool	getAvailableBlasters();
	bool	resetHardware	();
    bool	startReceive	(int portToUse, int timeout);
    bool	stopReceive		();
	void	sendToDecoder	(lirc_t *pData, int len);
	BOOL	transmit		(int *data, size_t dataLen, int transmitMask, int period);
	int		calcBlasterPort	();

	//==========================
	lirc_t			dataBuffer[256];
	UCHAR			bufferStart;
	UCHAR			bufferEnd;
	HANDLE			threadHandle;
	HANDLE			exitEvent;
	IrDeviceList	irDeviceList;
	HANDLE			deviceHandle;
	INT_TYPE		receivePort;
	UINT			transmitChannelMask;

	LARGE_INTEGER	time;
	LARGE_INTEGER	lastTime;
	LARGE_INTEGER	frequency;

	AvailableBlasters		availableBlasters;
	MCEDeviceCapabilities	mceDeviceCapabilities;
	//==========================
};

#endif