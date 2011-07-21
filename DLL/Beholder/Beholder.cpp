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
 * Copyright (C) 2011 Artem Golubev
 */

#include "Beholder.h"
#include "Globals.h"
#include <Windows.h>
#include "hardware.h"
#include "decode.h"

IG_API int init( HANDLE exitEvent )
{
	initHardwareStruct();

	InitializeCriticalSection(&criticalSection);

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent( NULL, TRUE, FALSE, NULL );

	sendReceiveData = new SendReceiveData();

	if( !sendReceiveData->init() ) {
      return 0;
	}

	return 1;
}

IG_API void deinit()
{
	if(sendReceiveData) {
		sendReceiveData->deinit();
		delete sendReceiveData;
		sendReceiveData = NULL;
	}

	if(dataReadyEvent) {
		CloseHandle( dataReadyEvent );
		dataReadyEvent = NULL;
	}

	DeleteCriticalSection(&criticalSection);

	threadExitEvent = NULL;
}

IG_API int hasGui()
{
	return FALSE;
}

IG_API void	loadSetupGui()
{
   // @TODO
}

IG_API int sendIR( struct ir_remote *remote, struct ir_ncode *code, int repeats )
{
	return 0;
}

IG_API int decodeIR( struct ir_remote *remotes, char *out )
{
	if(sendReceiveData) {

		sendReceiveData->waitTillDataIsReady( 0 );
	   	
		if(decodeCommand(remotes,out)) {
			return 1;
		}
	}

	return 0;
}

IG_API struct hardware* getHardware() {

	initHardwareStruct();
	return &hw;
}
