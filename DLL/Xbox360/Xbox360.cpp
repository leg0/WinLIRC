/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.9.0.
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
 * Copyright (C) 2011 Ian Curtis
 */

#include <Windows.h>
#include "SendReceive.h"
#include <winlirc/PluginApi.h>
#include <memory>

std::unique_ptr<SendReceive> sendReceive;

WL_API int init(winlirc_interface const* wl) {

	sendReceive = std::make_unique<SendReceive>();

	return sendReceive->init(CreateEvent(nullptr, TRUE, FALSE, nullptr));
}

WL_API void deinit() {

	if(sendReceive) {
		sendReceive->deinit();
		sendReceive.reset();
	}
}

WL_API int hasGui() {

	return FALSE;
}

WL_API void	loadSetupGui() {

}

WL_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return 0;
}

WL_API int decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(sendReceive) {
		using namespace std::chrono_literals;
		if(!sendReceive->waitTillDataIsReady(0us)) {
			return 0;
		}

		return sendReceive->decodeCommand(out, out_size);
	}

	return 0;
}