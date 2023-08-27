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

#include <stdio.h>
#include "Globals.h"
#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>

rbuf rec_buffer;

lirc_t readdata(lirc_t timeout) {

	if(!analyseAudio) return 0;

	waitTillDataIsReady(std::chrono::microseconds{ timeout });

	UINT data;
	if(analyseAudio->getData(&data)) {
		
		return static_cast<lirc_t>(data);
	}

	return 0;
}

int data_ready() {

	if(!analyseAudio) return 0;

	if(analyseAudio->dataReady()) return 1;

	return 0;
}

void wait_for_data(lirc_t uSecs) {

	waitTillDataIsReady(std::chrono::microseconds{ uSecs });
}

extern hardware const audio_hw = {
	.plugin_api_version = winlirc_plugin_api_version,
	.device        = "hw",
	.name          = "audio",
	.features      = LIRC_CAN_REC_MODE2,
	.send_mode     = 0,
	.rec_mode      = LIRC_MODE_MODE2,
	.code_length   = 0,
	.resolution    = 0,
	.decode_func   = &winlirc_receive_decode,
	.readdata      = &readdata,
	.wait_for_data = &wait_for_data,
	.data_ready    = &data_ready,
	.get_ir_code   = nullptr,
};
