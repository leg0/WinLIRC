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

#include <winsock2.h>
#include <Windows.h>
#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include "../Common/Win32Helpers.h"
#include <stdio.h>
#include "Globals.h"

extern hardware const udp_hw;
extern rbuf rec_buffer;

static int udp_init(winlirc_api const* winlirc) {

	//===========
	BOOL success;
	//===========

	winlirc_init_rec_buffer(&rec_buffer);

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);

	server = new Server();
	success = server->init();

	return success;
}

static void udp_deinit() {

	if(server) {
		server->deinit();
		delete server;
		server = nullptr;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

static int udp_hasGui() {

	return FALSE;
}

static void	udp_loadSetupGui() {

}

static int udp_sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return 0;
}

static int udp_decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(server) {
		using namespace std::chrono_literals;
		if(!server->waitTillDataIsReady(0us)) {
			return 0;
		}

		winlirc_clear_rec_buffer(&rec_buffer, &udp_hw);
		
		if(winlirc_decodeCommand(&rec_buffer, &udp_hw,remotes,out,out_size)) {
			return 1;
		}
	}

	return 0;
}

static hardware const* udp_getHardware() {

	return &udp_hw;
}

WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = udp_init,
		.deinit = udp_deinit,
		.hasGui = udp_hasGui,
		.loadSetupGui = udp_loadSetupGui,
		.sendIR = udp_sendIR,
		.decodeIR = udp_decodeIR,
		.getHardware = udp_getHardware,
		.hardware = &udp_hw,
	};
	return &p;
}
