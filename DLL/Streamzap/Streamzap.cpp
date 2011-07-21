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
#include "Streamzap.h"
#include "LIRCDefines.h"
#include <stdio.h>
#include "Globals.h"

IG_API int init(HANDLE exitEvent) {

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,TRUE,FALSE,NULL);

	streamzapAPI = new StreamzapAPI();

	return streamzapAPI->init();
}

IG_API void deinit() {

	if(streamzapAPI) {
		streamzapAPI->deinit();
		delete streamzapAPI;
		streamzapAPI = NULL;
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

	return 0;
}

IG_API int decodeIR(struct ir_remote *remotes, char *out) {

	if(streamzapAPI) {
		streamzapAPI->waitTillDataIsReady(0);
		return streamzapAPI->decodeCommand(out);
	}

	return 0;
}