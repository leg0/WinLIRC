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
#include <winlirc/WLPluginAPI.h>
#include "../Common/Win32Helpers.h"
#include <stdio.h>
#include "Globals.h"
#include <tchar.h>
#include "resource.h"

static int firefly_init(winlirc_api const* winlirc) {

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);

	sendReceiveData = new SendReceiveData();

	if(!sendReceiveData->init()) return 0;

	return 1;
}

static void firefly_deinit() {

	if(sendReceiveData) {
		sendReceiveData->deinit();
		delete sendReceiveData;
		sendReceiveData = nullptr;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

static int firefly_hasGui() {

	return FALSE;
}

static void	firefly_loadSetupGui() {

}

static int firefly_sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return 0;
}

static int firefly_decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(sendReceiveData) {
		using namespace std::chrono_literals;
		if(!sendReceiveData->waitTillDataIsReady(0us)) {
			return 0;
		}

		return sendReceiveData->decodeCommand(out, out_size);
	}

	return 0;
}

WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = firefly_init,
		.deinit = firefly_deinit,
		.hasGui = firefly_hasGui,
		.loadSetupGui = firefly_loadSetupGui,
		.sendIR = firefly_sendIR,
		.decodeIR = firefly_decodeIR,
		.getHardware = []() -> hardware const* { return nullptr; },
		.hardware = nullptr,
	};
	return &p;
}
