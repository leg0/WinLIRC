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

#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include <winlirc-common/Win32Helpers.h>

#include "Settings.h"
#include "Globals.h"
#include "SendReceiveData.h"
#include "winlirc.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
void initHardwareStruct();
extern hardware hw;
extern rbuf rec_buffer;
extern sbuf send_buffer;

WL_API int init(WLEventHandle exitEvent) {

	//==========
	int success;
	//==========

	winlirc_init_rec_buffer		(&rec_buffer);
	winlirc_init_send_buffer	(&send_buffer);
	initHardwareStruct	();

	threadExitEvent = reinterpret_cast<HANDLE>(exitEvent);
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);

	success = init_commandir();

	return success;
}

WL_API void deinit() {

	deinit_commandir();

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

WL_API int hasGui() {

	return FALSE;
}

WL_API void	loadSetupGui() {

}

WL_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return send(remote,code,repeats);
}

WL_API int decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(!waitTillDataIsReady(0)) {
		return 0;
	}

	winlirc_clear_rec_buffer(&rec_buffer, &hw);
	
	if(winlirc_decodeCommand(&rec_buffer, &hw,remotes,out,out_size)) {
		return 1;
	}

	return 0;
}

WL_API int setTransmitters(unsigned int transmitterMask) {

	currentTransmitterMask = transmitterMask;

	return 1;	// assume success ... for now :p
}

WL_API hardware const* getHardware() {

	initHardwareStruct();
	return &hw;

}