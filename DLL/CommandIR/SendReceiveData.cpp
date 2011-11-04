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
#include "Settings.h"
#include "Globals.h"
#include <stdio.h>
#include "Send.h"
#include "usb.h"

#include "guicon.h"

#include "winlirc.h"

extern "C" {
	extern int check_commandir_rec(void);
	extern void send_lirc_buffer(unsigned char *buffer, int bytes, unsigned int frequency);
	extern void initForWinLIRC();
	extern void check_commandir_add_remove();
	extern lirc_t		dataBuffer[256];
	extern UCHAR		bufferStart;
	extern UCHAR		bufferEnd;
}

#include "CommandIR.h"
extern IG_API int setTransmitters(unsigned int transmitterMask);


SendReceiveData::SendReceiveData() {

	bufferStart		= 0;
	bufferEnd		= 0;
}

bool SendReceiveData::init() {

	//
	// init your device here
	//

	initForWinLIRC();

	return true;
}

void SendReceiveData::deinit() {


}

/* Rewritten to avoid 2nd thread and signals */
void SendReceiveData::waitTillDataIsReady(int maxUSecs) {
	int rxed = 0;
	
	/* We want to return sometimes so we can still send? */
	while( bufferStart==bufferEnd ) {
		rxed = check_commandir_rec();
		if(rxed > 0) {
			break;	// force abreak
		} else {
			Sleep(maxUSecs > 50000 ? maxUSecs / 1000 : 50);
			check_commandir_add_remove();
		}
	}
}

/* We're not using this */
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

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}



//======================================================================================
// sending stuff below
//======================================================================================

int SendReceiveData::send(ir_remote *remote, ir_ncode *code, int repeats) {
	int frequency = 38000;
	static int testTransmitters = 1;
	
	if(remote->freq)
		frequency = remote->freq;

	if (init_send(remote, code,repeats))
	{
		//==================
		int		length;
		lirc_t	*signals;
		//==================

		length		= send_buffer.wptr;
		signals		= send_buffer.data;

		//
		// raw array of timing values
		// send to your device for sending
		//

		if (length <= 0 || signals == NULL) {
			return 0;
		}

		// How are emitters set?
		// with this new function i added :p
//		setTransmitters(5);
		setTransmitters(testTransmitters);
		printf("Sending %d signals at %dhz with transmitters at %2X\n", length, frequency, testTransmitters);

		testTransmitters++;
		if(testTransmitters > 0xff)
				testTransmitters = 1;
		send_lirc_buffer((unsigned char*)signals, length * sizeof(lirc_t), frequency);	// send_lirc_buffer wants BYTES in buffer.
		return length;
	}

	return 0;	// for not implimented
}

