#include "Globals.h"
#include <stdio.h>
#include "LIRCDefines.h"
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

#include "Serial.h"
#include "SerialDialog.h"
#include "irdriver.h"
#include "Decode.h"
#include "hardware.h"
#include "Send.h"


SI_API int init(HANDLE exitEvent) {

	threadExitEvent	= exitEvent;

	initHardwareStruct();
	init_rec_buffer();

	irDriver = new CIRDriver();
	if(irDriver->InitPort()) return 1;

	return 0;
}

SI_API void deinit() {

	threadExitEvent = NULL;	//this one is created outside the DLL

	if(irDriver) {
		delete irDriver;
		irDriver = NULL;
	}
}

SI_API int hasGui() {

	return TRUE;
}

SI_API void loadSetupGui() {

	SerialDialog serialDialog;
	serialDialog.DoModal();
}

SI_API int sendIR(struct ir_remote *remotes, struct ir_ncode *code, int repeats) {

	//
	// This function is totally untested since i have no hardware to test it
	// By some miracle it might work
	//

	send(code,remotes,repeats);

	return 1;
}

SI_API int decodeIR(struct ir_remote *remotes, char *out) {

	if(irDriver) {
		irDriver->waitTillDataIsReady(0);
	}

	if(decodeCommand(remotes, out)) {
		return 1;
	}
	
	return 0;
}

SI_API struct hardware* getHardware() {

	initHardwareStruct();	//make sure values are setup

	return &hw;
}