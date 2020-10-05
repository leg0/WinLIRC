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

#include "Globals.h"
#include <Windows.h>

#include <winlirc/WLPluginAPI.h>
#include <winlirc/IRRemote.h>

void initHardwareStruct();
extern hardware hw;

WL_API int init( WLEventHandle exitEvent )
{
	initHardwareStruct();

	InitializeCriticalSection(&criticalSection);

	threadExitEvent = reinterpret_cast<HANDLE>(exitEvent);
	dataReadyEvent	= CreateEvent( nullptr, TRUE, FALSE, nullptr );

	sendReceiveData = new SendReceiveData();

	if( !sendReceiveData->init() ) {
      return 0;
	}

	return 1;
}

WL_API void deinit()
{
	if(sendReceiveData) {
		sendReceiveData->deinit();
		delete sendReceiveData;
		sendReceiveData = nullptr;
	}

	if(dataReadyEvent) {
		CloseHandle( dataReadyEvent );
		dataReadyEvent = nullptr;
	}

	DeleteCriticalSection(&criticalSection);

	threadExitEvent = nullptr;
}

WL_API int hasGui()
{
	return FALSE;
}

WL_API void	loadSetupGui()
{
   // @TODO
}

WL_API int sendIR( struct ir_remote *remote, struct ir_ncode *code, int repeats )
{
	return 0;
}

WL_API int decodeIR( struct ir_remote *remotes, char *out, size_t out_size )
{
	if(sendReceiveData) {

		using namespace std::chrono_literals;
		if(!sendReceiveData->waitTillDataIsReady(0us)) {
			return 0;
		}
	   	
		if(winlirc_decodeCommand(&hw,remotes,out,out_size)) {
			ResetEvent(dataReadyEvent);
			return 1;
		}

		ResetEvent(dataReadyEvent);
	}

	return 0;
}

WL_API hardware const* getHardware() {

	initHardwareStruct();
	return &hw;
}
