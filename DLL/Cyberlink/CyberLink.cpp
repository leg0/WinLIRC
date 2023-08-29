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

extern hardware const cyberlink_hw;
extern rbuf rec_buffer;

static int cyberlink_init(winlirc_api const* winlirc)
{
	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);

	receiveData = new ReceiveData();

	if(!receiveData->init()) return 0;

	return 1;
}

static void cyberlink_deinit()
{
	if ( receiveData ) {
		receiveData->deinit();
		delete receiveData;
		receiveData = nullptr;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

static int cyberlink_hasGui()
{
	return FALSE;
}

static void	cyberlink_loadSetupGui()
{
}

static int cyberlink_sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats)
{
	//
	// return false - we don't support this function
	//
	return 0;
}

static int cyberlink_decodeIR(struct ir_remote *remotes, char *out, size_t out_size)
{
	if ( receiveData ) {
		using namespace std::chrono_literals;
		if ( !receiveData->waitTillDataIsReady(0us) ) {
			return 0;
		}

		receiveData->getData((lirc_t*)&irCode);

		if (winlirc_decodeCommand(&rec_buffer, &cyberlink_hw,remotes, out, out_size) ) {
			return 1;
		}
	}

	return 0;
}

static hardware const* cyberlink_getHardware()
{
	return &cyberlink_hw;
}

WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = cyberlink_init,
		.deinit = cyberlink_deinit,
		.hasGui = cyberlink_hasGui,
		.loadSetupGui = cyberlink_loadSetupGui,
		.sendIR = cyberlink_sendIR,
		.decodeIR = cyberlink_decodeIR,
		.getHardware = cyberlink_getHardware,
		.hardware = &cyberlink_hw,
	};
	return &p;
}
