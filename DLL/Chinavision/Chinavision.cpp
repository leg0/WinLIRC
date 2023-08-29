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
#include "Chinavision.h"
#include <stdio.h>
#include "ChinavisionAPI.h"

ChinavisionAPI *chinavisionAPI = nullptr;

static int chinavision_init(winlirc_api const* winlirc) {

	chinavisionAPI = new ChinavisionAPI();
	return chinavisionAPI->init(reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc)));
}

static void chinavision_deinit() {

	if(chinavisionAPI) {
		chinavisionAPI->deinit();
		delete chinavisionAPI;
		chinavisionAPI = nullptr;
	}
}

static int chinavision_hasGui() {

	return FALSE;
}

static void	chinavision_loadSetupGui() {

}

static int chinavision_sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return 0;
}

static int chinavision_decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(chinavisionAPI) {
		using namespace std::chrono_literals;
		chinavisionAPI->waitTillDataIsReady(0us);
		return chinavisionAPI->decodeCommand(out, out_size);
	}

	return 0;
}


WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = chinavision_init,
		.deinit = chinavision_deinit,
		.hasGui = chinavision_hasGui,
		.loadSetupGui = chinavision_loadSetupGui,
		.sendIR = chinavision_sendIR,
		.decodeIR = chinavision_decodeIR,
		.getHardware = []() -> hardware const* { return nullptr; },
		.hardware = nullptr,
	};
	return &p;
}
