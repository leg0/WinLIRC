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
#include "iguanaIR.h"
#include "SendReceiveData.h"
#include "Settings.h"
#include "Globals.h"
#include <stdio.h>
#include "Send.h"

DWORD WINAPI IGThread(void *recieveClass) {

	((SendReceiveData*)recieveClass)->threadProc();
	return 0;
}

SendReceiveData::SendReceiveData() {

	bufferStart		= 0;
	bufferEnd		= 0;
	recvDone		= 0;
	connection		= INVALID_PIPE;
	sendConnection	= INVALID_PIPE;
	threadHandle	= NULL;
	currentCarrier	= -1;
}

bool SendReceiveData::init() {

	//======================
	char deviceNumber[1024];
	//======================

	sprintf(deviceNumber,"%i",settings.getDeviceNumber());

	connection		= iguanaConnect(deviceNumber);
	sendConnection	= iguanaConnect(deviceNumber);

	if (connection == INVALID_PIPE || sendConnection == INVALID_PIPE) {
		return false;
	}

	threadHandle = CreateThread(NULL,0,IGThread,(void *)this,0,NULL);

	recvDone = 0;

	if(threadHandle) {
		return true;
	}

	return false;
}

void SendReceiveData::deinit() {

	recvDone = 1;

	killThread();

	if(connection!=INVALID_PIPE) {
		iguanaClose(connection);
		connection=INVALID_PIPE;
	}

	if(sendConnection!=INVALID_PIPE) {
		iguanaClose(sendConnection);
		sendConnection=INVALID_PIPE;
	}
}


void SendReceiveData::threadProc() {

	receiveLoop();
}

void SendReceiveData::killThread() {

	while(threadHandle!=NULL) {

		//===========
		DWORD result;
		//===========

		result = 0;

		if(GetExitCodeThread(threadHandle,&result)==0) 
		{
			return;
		}

		if(result==STILL_ACTIVE)
		{
			if(WAIT_TIMEOUT==WaitForSingleObject(threadHandle,INFINITE)) break;

			threadHandle=NULL;
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

	if (connection != INVALID_PIPE)
	{
		iguanaPacket request, response;
		lirc_t prevCode = -1;

		request = iguanaCreateRequest(IG_DEV_RECVON, 0, NULL);
		if (iguanaWriteRequest(request, connection))
			while(! recvDone)
			{
				/* read from device */
				do
				{
					response = iguanaReadResponse(connection, 1000);
				}
				while (!recvDone &&
					((response == NULL)
					|| (iguanaResponseIsError(response))));

				if (iguanaResponseIsError(response))
				{
					/* be quiet during exit */

					break;
				}
				else if (iguanaCode(response) == IG_DEV_RECV)
				{
					lirc_t *code;
					unsigned int length, x, y = 0;
					lirc_t buffer[8]; /* we read 8 bytes max at a time
							   * from the device, i.e. packet
							   * can only contain 8
							   * signals. */

					/* pull the data off the packet */
					code = (lirc_t*)iguanaRemoveData(response, &length);
					length /= sizeof(lirc_t);

					/* translate the code into lirc_t pulses (and make
					 * sure they don't split across iguana packets. */
					for(x = 0; x < length; x++)
					{
						if (prevCode == -1)
						{
							prevCode = (code[x] & IG_PULSE_MASK);
							if(prevCode > PULSE_MASK) prevCode = PULSE_MASK;
							if(code[x] & IG_PULSE_BIT) prevCode |= PULSE_BIT;
						}
						else if (((prevCode & PULSE_BIT) && (code[x]  & IG_PULSE_BIT)) ||
							 (!(prevCode & PULSE_BIT) && !(code[x]  & IG_PULSE_BIT)))
						{
							/* can overflow pulse mask, so just set to
							 * largest possible */
							if ((prevCode & PULSE_MASK) + (code[x] & IG_PULSE_MASK) >
							    PULSE_MASK)
								prevCode = (prevCode & PULSE_BIT) | PULSE_MASK;
							else
								prevCode += code[x] & IG_PULSE_MASK;
						}
						else
						{
							buffer[y] = prevCode;
							y++;

							prevCode = (code[x] & IG_PULSE_MASK);
							if(prevCode > PULSE_MASK) prevCode = PULSE_MASK;
							if(code[x] & IG_PULSE_BIT) prevCode |= PULSE_BIT;
						}
					}

					/* write the data and free it */

					for(unsigned int i=0; i<y; i++) {
						setData(buffer[i]);
					}

					if(y) SetEvent(dataReadyEvent);

					free(code);
				}

				iguanaFreePacket(response);
			}

		iguanaFreePacket(request);
	}

	iguanaClose(connection);
	connection = INVALID_PIPE;
}

//======================================================================================
// sending stuff below
//======================================================================================

bool SendReceiveData::daemonTransaction(unsigned char code, void *value, size_t size)
{
	//=============
	UCHAR	*data;
	bool	retval;
	//=============

	retval = false;

	data = (UCHAR*)malloc(size);

	if (data != NULL)
	{
		//====================
		iguanaPacket request;
		iguanaPacket response;
		//====================

		request		= NULL;
		response	= NULL;

		memcpy(data, value, size);

		request = iguanaCreateRequest(code, size, data);

		if (request)
		{
			if (iguanaWriteRequest(request, sendConnection)) {
				response = iguanaReadResponse(sendConnection, 10000);
			}

			iguanaFreePacket(request);
		}
		else {
			free(data);
		}

		/* handle success */
		if (! iguanaResponseIsError(response)) {
			retval = true;
		}

		iguanaFreePacket(response);
	}

	return retval;
}

bool SendReceiveData::setTransmitters(UCHAR channels) {

	//
	// santity check
	//
	if(channels>15) channels = 15;
	
	return daemonTransaction(IG_DEV_SETCHANNELS, &channels, sizeof(channels));
}

int SendReceiveData::send(ir_remote *remote, ir_ncode *code, int repeats) {

	//=============
	int		retval;
	UINT	freq;
	//=============

	retval = 0;

	/* set the carrier frequency if necessary */
	freq = htonl(remote->freq);

	if (remote->freq != currentCarrier 
	&& remote->freq >= 25000 && remote->freq <= 100000 
	&& daemonTransaction(IG_DEV_SETCARRIER, &freq, sizeof(freq))) {

		currentCarrier = remote->freq;
	}

	if (init_send(remote, code,repeats))
	{
		//==================
		int		length;
		int		x;
		lirc_t	*signals;
		UINT	*igsignals;
		//==================

		length		= send_buffer.wptr;
		signals		= send_buffer.data;
		igsignals	= (UINT*)malloc(sizeof(UINT) * length);

		if (igsignals != NULL)
		{
			iguanaPacket request, response = NULL;

			/* must pack the data into a unit32_t array */
			for(x = 0; x < length; x++)
			{
				igsignals[x] = signals[x] & PULSE_MASK;

				if (signals[x] & PULSE_BIT) {
					igsignals[x] |= IG_PULSE_BIT;
				}
			}

			/* construct a request and send it to the daemon */
			request = iguanaCreateRequest(IG_DEV_SEND,sizeof(UINT) * length, igsignals);

			if (iguanaWriteRequest(request, sendConnection))
			{
				/* response will only come back after the device has transmitted */
				response = iguanaReadResponse(sendConnection, 10000);

				if (! iguanaResponseIsError(response))
				{
					retval = 1;
				}

				iguanaFreePacket(response);
			}

			/* free the packet and the data */
			iguanaFreePacket(request);
		}

	}

	return retval;
}

