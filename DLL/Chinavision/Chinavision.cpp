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

WL_API int init(WLEventHandle exitEvent) {

	chinavisionAPI = new ChinavisionAPI();
	return chinavisionAPI->init(reinterpret_cast<HANDLE>(exitEvent));
}

WL_API void deinit() {

	if(chinavisionAPI) {
		chinavisionAPI->deinit();
		delete chinavisionAPI;
		chinavisionAPI = nullptr;
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

WL_API int decodeIR(struct ir_remote *remotes, size_t remotes_count, char *out, size_t out_size) {

	if(chinavisionAPI) {
		using namespace std::chrono_literals;
		chinavisionAPI->waitTillDataIsReady(0us);
		return chinavisionAPI->decodeCommand(out, out_size);
	}

	return 0;
}