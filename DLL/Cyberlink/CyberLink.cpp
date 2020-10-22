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
 * Copyright (C) 2013 Jan Dubiec <jdx(at)onet(dot)pl>
 */

#include <Windows.h>
#include <winlirc/WLPluginAPI.h>
#include <winlirc/winlirc_api.h>
#include "../Common/Win32Helpers.h"
#include "Globals.h"
#include "ReceiveData.h"

void initHardwareStruct();
extern hardware hw;

WL_API int init(WLEventHandle exitEvent)
{
	initHardwareStruct();

	threadExitEvent = reinterpret_cast<HANDLE>(exitEvent);
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);

	receiveData = new ReceiveData();

	if(!receiveData->init()) return 0;

	return 1;
}

WL_API void deinit()
{
	if ( receiveData ) {
		receiveData->deinit();
		delete receiveData;
		receiveData = nullptr;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

WL_API int hasGui()
{
	return FALSE;
}

WL_API void	loadSetupGui()
{
}

WL_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats)
{
	//
	// return false - we don't support this function
	//
	return 0;
}

WL_API int decodeIR(struct ir_remote *remotes, char *out, size_t out_size)
{
	if ( receiveData ) {
		using namespace std::chrono_literals;
		if ( !receiveData->waitTillDataIsReady(0us) ) {
			return 0;
		}

		receiveData->getData((lirc_t*)&irCode);

		if (winlirc_decodeCommand(&hw,remotes, out, out_size) ) {
			return 1;
		}
	}

	return 0;
}

WL_API hardware const* getHardware()
{
	initHardwareStruct();
	return &hw;
}
