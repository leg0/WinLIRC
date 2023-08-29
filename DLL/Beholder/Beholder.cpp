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
#include <winlirc/winlirc_api.h>

extern hardware const beholder_hw;
extern rbuf rec_buffer;

static int beholder_init(winlirc_api const* winlirc)
{
	InitializeCriticalSection(&criticalSection);

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent	= CreateEvent( nullptr, TRUE, FALSE, nullptr );

	sendReceiveData = new SendReceiveData();

	if( !sendReceiveData->init() ) {
      return 0;
	}

	return 1;
}

static void beholder_deinit()
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

static int beholder_hasGui()
{
	return FALSE;
}

static void	beholder_loadSetupGui()
{
   // @TODO
}

static int beholder_sendIR( struct ir_remote *remote, struct ir_ncode *code, int repeats )
{
	return 0;
}

static int beholder_decodeIR( struct ir_remote *remotes, char *out, size_t out_size )
{
	if(sendReceiveData) {

		using namespace std::chrono_literals;
		if(!sendReceiveData->waitTillDataIsReady(0us)) {
			return 0;
		}
	   	
		if(winlirc_decodeCommand(&rec_buffer, &beholder_hw,remotes,out,out_size)) {
			ResetEvent(dataReadyEvent);
			return 1;
		}

		ResetEvent(dataReadyEvent);
	}

	return 0;
}

static hardware const* beholder_getHardware() {

	return &beholder_hw;
}

WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = beholder_init,
		.deinit = beholder_deinit,
		.hasGui = beholder_hasGui,
		.loadSetupGui = beholder_loadSetupGui,
		.sendIR = beholder_sendIR,
		.decodeIR = beholder_decodeIR,
		.getHardware = beholder_getHardware,
		.hardware = &beholder_hw,
	};
	return &p;
}
