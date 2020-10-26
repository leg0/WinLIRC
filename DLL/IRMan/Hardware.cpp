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

#include "Globals.h"
#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include <chrono>

#define CODE_LENGTH 64

static int irman_decode (rbuf* rec_buffer, hardware const*,ir_remote *remote, ir_code *prep, ir_code *codep,
		 ir_code *postp, int *repeat_flagp,
		 lirc_t *min_remaining_gapp,
		 lirc_t *max_remaining_gapp)
{
	//==========
	int success;
	//==========

	success = 0;

	EnterCriticalSection(&criticalSection);

	success = winlirc_map_code(remote, prep, codep, postp, 0, 0, CODE_LENGTH, irCode, 0, 0);

	LeaveCriticalSection(&criticalSection);

	if(!success) return 0;

	using namespace std::chrono;
	winlirc_map_gap(remote, duration_cast<microseconds>(last-start).count(), 0, repeat_flagp,min_remaining_gapp, max_remaining_gapp);
	
	return 1;
}

ir_code irman_get_ir_code() {

	//==================
	ir_code currentCode;
	//==================

	EnterCriticalSection(&criticalSection);
	currentCode = irCode;
	ResetEvent(dataReadyEvent);
	LeaveCriticalSection(&criticalSection);

	return currentCode;
}

void wait_for_data(lirc_t timeout) {

	if(!sendReceiveData) return;

	sendReceiveData->waitTillDataIsReady(std::chrono::microseconds{ timeout });
}

int data_ready() {

	if(!sendReceiveData) return 0;

	return sendReceiveData->dataReady();
}

hardware hw;
rbuf rec_buffer;

void initHardwareStruct() {

	hw.decode_func	= &irman_decode;
	hw.readdata		= nullptr;
	hw.wait_for_data= &wait_for_data;
	hw.data_ready	= &data_ready;
	hw.get_ir_code	= &irman_get_ir_code;

	hw.features		= LIRC_CAN_REC_LIRCCODE;
	hw.send_mode	= 0;
	hw.rec_mode		= LIRC_MODE_LIRCCODE;
	hw.code_length	= 64;
	hw.resolution	= 0;

	strcpy_s(hw.device,"hw");
	strcpy_s(hw.name,"irman");
}