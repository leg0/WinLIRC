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

#include <Windows.h>
#include "CommandIR.h"
#include "Settings.h"
#include "Globals.h"
#include "Decode.h"
#include "LIRCDefines.h"
#include <stdio.h>
#include "SendReceiveData.h"
#include "hardware.h"
#include "Send.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
EXTERN_C int setCommandIRTransmitters(unsigned int transmitterMask);

IG_API int init(HANDLE exitEvent) {

	init_rec_buffer();
	init_send_buffer();
	initHardwareStruct();

	sendReceiveData = new SendReceiveData();

	if(!sendReceiveData->init()) return 0;

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);

	return 1;
}

IG_API void deinit() {

	if(sendReceiveData) {
		sendReceiveData->deinit();
		delete sendReceiveData;
		sendReceiveData = NULL;
	}

	if(dataReadyEvent) {
		CloseHandle(dataReadyEvent);
		dataReadyEvent = NULL;
	}

	threadExitEvent = NULL;

}

IG_API int hasGui() {

	return FALSE;
}

IG_API void	loadSetupGui() {

}

IG_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	if(sendReceiveData) {
		return sendReceiveData->send(remote,code,repeats);
	}

	return 0;
}

IG_API int decodeIR(struct ir_remote *remotes, char *out) {

	if(sendReceiveData) {
		sendReceiveData->waitTillDataIsReady(0);
	}

	if(decodeCommand(remotes,out)) {
		return 1;
	}

	return 0;
}

IG_API int setTransmitters(unsigned int transmitterMask) {

	//
	// transmitters received here !
	// return 1 for transmitter mask success 0 for fail
	// each bit reprisents a transmitter
	// so the binary value 0000001 = transmitter 1
	// value 00000011 = transmitters 1 & 2
	//
	setCommandIRTransmitters(transmitterMask);
	return 0;
}

IG_API struct hardware* getHardware() {

	initHardwareStruct();
	return &hw;

}