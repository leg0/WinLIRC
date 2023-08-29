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
#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include <stdio.h>
#include "Globals.h"

extern hardware const streamzap_hw;
extern rbuf rec_buffer;

static int streamzap_init(winlirc_api const* winlirc) {

	winlirc_init_rec_buffer(&rec_buffer);

	streamzapAPI = new StreamzapAPI();

	return streamzapAPI->init(reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc)));
}

static void streamzap_deinit() {

	if(streamzapAPI) {
		streamzapAPI->deinit();
		delete streamzapAPI;
		streamzapAPI = nullptr;
	}
}

static int streamzap_hasGui() {

	return FALSE;
}

static void	streamzap_loadSetupGui() {

}

static int streamzap_sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return 0;
}

static int streamzap_decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(streamzapAPI) {
		using namespace std::chrono_literals;
		if(!streamzapAPI->waitTillDataIsReady(0us)) {
			return 0;
		}

		winlirc_clear_rec_buffer(&rec_buffer, &streamzap_hw);
		
		if(winlirc_decodeCommand(&rec_buffer, &streamzap_hw,remotes,out, out_size)) {
			return 1;
		}
	}

	return 0;
}

static hardware const* streamzap_getHardware() {
	return &streamzap_hw;
}

WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = streamzap_init,
		.deinit = streamzap_deinit,
		.hasGui = streamzap_hasGui,
		.loadSetupGui = streamzap_loadSetupGui,
		.sendIR = streamzap_sendIR,
		.decodeIR = streamzap_decodeIR,
		.getHardware = streamzap_getHardware,
		.hardware = &streamzap_hw,
	};
	return &p;
}
