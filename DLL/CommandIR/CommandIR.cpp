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
#include "../Common/Win32Helpers.h"

#include "Settings.h"
#include "Globals.h"
#include "SendReceiveData.h"
#include "winlirc.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
extern hardware const commandir_hw;
extern rbuf rec_buffer;
extern sbuf send_buffer;

static int commandir_init(winlirc_api const* winlirc) {

	//==========
	int success;
	//==========

	winlirc_init_rec_buffer		(&rec_buffer);
	winlirc_init_send_buffer	(&send_buffer);

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);

	success = init_commandir();

	return success;
}

static void commandir_deinit() {

	deinit_commandir();

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

static int commandir_hasGui() {

	return FALSE;
}

static void	commandir_loadSetupGui() {

}

static int commandir_sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return send(remote,code,repeats);
}

static int commandir_decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(!waitTillDataIsReady(0)) {
		return 0;
	}

	winlirc_clear_rec_buffer(&rec_buffer, &commandir_hw);
	
	if(winlirc_decodeCommand(&rec_buffer, &commandir_hw,remotes,out,out_size)) {
		return 1;
	}

	return 0;
}

static int commandir_setTransmitters(unsigned int transmitterMask) {

	currentTransmitterMask = transmitterMask;

	return 1;	// assume success ... for now :p
}

static hardware const* commandir_getHardware() {

	return &commandir_hw;

}

WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = commandir_init,
		.deinit = commandir_deinit,
		.hasGui = commandir_hasGui,
		.loadSetupGui = commandir_loadSetupGui,
		.sendIR = commandir_sendIR,
		.decodeIR = commandir_decodeIR,
		.getHardware = commandir_getHardware,
		.hardware = &commandir_hw,
	};
	return &p;
}
