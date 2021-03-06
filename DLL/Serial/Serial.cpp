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

#include "Globals.h"
#include <stdio.h>
#include "SerialDialog.h"
#include "irdriver.h"

#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include "Transmit.h"

void initHardwareStruct();
extern hardware hw;
extern rbuf rec_buffer;

WL_API int init(WLEventHandle exitEvent) {

	threadExitEvent	= reinterpret_cast<HANDLE>(exitEvent);

	initHardwareStruct();
	winlirc_init_rec_buffer(&rec_buffer);

	irDriver = new CIRDriver();
	if(irDriver->InitPort()) return 1;

	return 0;
}

WL_API void deinit() {

	threadExitEvent = nullptr;	//this one is created outside the DLL

	if(irDriver) {
		delete irDriver;
		irDriver = nullptr;
	}
}

WL_API int hasGui() {

	return TRUE;
}

WL_API void loadSetupGui() {

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	SerialDialog serialDialog;
	serialDialog.DoModal();
}

WL_API int sendIR(struct ir_remote *remotes, struct ir_ncode *code, int repeats) {

	return Transmit(code,remotes,repeats);
}

WL_API int decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(irDriver) {
		using namespace std::chrono_literals;
		if(!irDriver->waitTillDataIsReady(0us)) {
			return 0;
		}
	}

	winlirc_clear_rec_buffer(&rec_buffer, &hw);

	if(winlirc_decodeCommand(&rec_buffer, &hw, remotes, out, out_size)) {
		return 1;
	}
	
	return 0;
}

WL_API hardware const* getHardware() {

	initHardwareStruct();	//make sure values are setup

	return &hw;
}