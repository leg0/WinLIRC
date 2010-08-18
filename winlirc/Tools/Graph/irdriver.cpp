/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
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
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * Modifications Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#include "irdriver.h"
	
CIRDriver::CIRDriver()
{
	initFunction			= NULL;
	deinitFunction			= NULL;
	hasGuiFunction			= NULL;
	loadSetupGuiFunction	= NULL;
	sendFunction			= NULL;
	decodeFunction			= NULL;
	getHardwareFunction		= NULL;

	dllFile					= NULL;
}

CIRDriver::~CIRDriver()
{
	unloadPlugin();
}

BOOL CIRDriver::loadPlugin(CString plugin) {

	//
	//make sure we have cleaned up first
	//
	unloadPlugin();

	loadedPlugin	= plugin;
	dllFile			= LoadLibrary(plugin);

	if(!dllFile) return FALSE;

	initFunction			= (InitFunction)		GetProcAddress(dllFile,"init");
	deinitFunction			= (DeinitFunction)		GetProcAddress(dllFile,"deinit");
	hasGuiFunction			= (HasGuiFunction)		GetProcAddress(dllFile,"hasGui");
	loadSetupGuiFunction	= (LoadSetupGuiFunction)GetProcAddress(dllFile,"loadSetupGui");
	sendFunction			= (SendFunction)		GetProcAddress(dllFile,"sendIR");
	decodeFunction			= (DecodeFunction)		GetProcAddress(dllFile,"decodeIR");
	getHardwareFunction		= (GetHardware)			GetProcAddress(dllFile,"getHardware");

	if(initFunction && deinitFunction && hasGuiFunction && loadSetupGuiFunction && sendFunction && decodeFunction) {
		return TRUE;
	}

	return FALSE;
}

void CIRDriver::unloadPlugin() {

	//
	// make sure we have cleaned up
	//
	deinit();

	initFunction			= NULL;
	deinitFunction			= NULL;
	hasGuiFunction			= NULL;
	loadSetupGuiFunction	= NULL;
	sendFunction			= NULL;
	decodeFunction			= NULL;
	getHardwareFunction		= NULL;

	if(dllFile) {
		FreeLibrary(dllFile);
	}

	dllFile					= NULL;
}

BOOL CIRDriver::init() {

	//
	// safe to call deinit first
	//
	deinit();

	if(initFunction) {
		if(initFunction(0)) {

			return TRUE;
		}
	}

	return FALSE;
}

void CIRDriver::deinit() {

	if(deinitFunction) {
		deinitFunction();
	}

	//KillThread2(&daemonThreadHandle,daemonThreadEvent);
}

int	CIRDriver::sendIR(struct ir_remote *remote,struct ir_ncode *code, int repeats) {

	if(sendFunction) {
		return sendFunction(remote,code,repeats);
	}

	return 0;
}

int	CIRDriver::decodeIR(struct ir_remote *remote, char *out) {

	if(decodeFunction) {
		return decodeFunction(remote,out);
	}

	return 0;
}

struct hardware* CIRDriver::getHardware() {

	if(!getHardwareFunction) return NULL;

	return getHardwareFunction();
}