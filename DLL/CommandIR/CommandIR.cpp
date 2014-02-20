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
#include <stdio.h>

#include "../Common/LIRCDefines.h"
#include "../Common/IRRemote.h"
#include "../Common/Receive.h"
#include "../Common/Hardware.h"
#include "../Common/Send.h"
#include "../Common/WLPluginAPI.h"
#include "../Common/Win32Helpers.h"

#include "Settings.h"
#include "Globals.h"
#include "SendReceiveData.h"
#include "winlirc.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

WL_API int init(HANDLE exitEvent) {

	//==========
	int success;
	//==========

	init_rec_buffer		();
	init_send_buffer	();
	initHardwareStruct	();

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,TRUE,FALSE,NULL);

	success = init_commandir();

	return success;
}

WL_API void deinit() {

	deinit_commandir();

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = NULL;
}

WL_API int hasGui() {

	return FALSE;
}

WL_API void	loadSetupGui() {

}

WL_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return send(remote,code,repeats);
}

WL_API int decodeIR(struct ir_remote *remotes, char *out) {

	if(!waitTillDataIsReady(0)) {
		return 0;
	}

	clear_rec_buffer();
	
	if(decodeCommand(remotes,out)) {
		return 1;
	}

	return 0;
}

WL_API int setTransmitters(unsigned int transmitterMask) {

	currentTransmitterMask = transmitterMask;

	return 1;	// assume success ... for now :p
}

WL_API struct hardware* getHardware() {

	initHardwareStruct();
	return &hw;

}