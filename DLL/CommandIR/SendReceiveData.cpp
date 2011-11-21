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

lirc_t			dataBuffer[256];
unsigned char	bufferStart;
unsigned char	bufferEnd;

void waitTillDataIsReady(int maxUSecs) {

	//================
	HANDLE	events[2];
	int		evt;
	//================

	events[0] = dataReadyEvent;
	events[1] = threadExitEvent;

	if( threadExitEvent==NULL ) {
		evt = 1;
	}
	else {
      evt = 2;
	}

	if(!dataReady()) {

		//======
		int res;
		//======
		ResetEvent(dataReadyEvent);

		if( maxUSecs ) {
			res = WaitForMultipleObjects( evt, events, FALSE, ( maxUSecs + 500 ) / 1000 );
		}
		else {
			res = WaitForMultipleObjects( evt, events, FALSE, INFINITE );
		}

		if( res == (WAIT_OBJECT_0+1) ) {
			ExitThread( 0 );
			return;
		}

	}
}

void setData(lirc_t data) {

	//printf("data %i\n",data);
	dataBuffer[bufferEnd] = data;
	bufferEnd++;

	SetEvent( dataReadyEvent );	// tell thread data is ready :p
}

/* Replacing LIRC's function with one for libcmdir to call from C */
extern "C" void lirc_pipe_write( int * data )
{
	// hw seems to be returning zeros at the end of the signal - we dont want those :p
	if(*data) {	 //-- Actually, this encourages LIRC to decode a signal without waiting for space/timeout MB
		setData(*data);
	}
}

bool dataReady() {

	if(bufferStart==bufferEnd) {
		return false;
	}
	return true;
}

bool getData(lirc_t *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}



//======================================================================================
// sending stuff below
//======================================================================================


int send(ir_remote *remote, ir_ncode *code, int repeats) {

	int frequency = 38000;
	
	if(remote->freq) {
		frequency = remote->freq;
	}

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

		printf("Sending %d signals at %dhz with transmitters at %2X\n", length, frequency, currentTransmitterMask);

		send_lirc_buffer((unsigned char*)signals, length * sizeof(lirc_t), frequency);	// send_lirc_buffer wants BYTES in buffer.

		return 1; // assume success :p
	}

	return 0;	// for not implimented
}


