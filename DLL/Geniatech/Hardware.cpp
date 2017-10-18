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
#include "../Common/LIRCDefines.h"
#include "../Common/Hardware.h"
#include "../Common/IRRemote.h"

#define CODE_LENGTH 32

int gnt_decode (struct hardware const*,struct ir_remote *remote, ir_code *prep, ir_code *codep,
		 ir_code *postp, int *repeat_flagp,
		 lirc_t *min_remaining_gapp,
		 lirc_t *max_remaining_gapp)
{
	//==========
	int success;
	//==========

	success = 0;

	success = map_code(remote, prep, codep, postp, 0, 0, CODE_LENGTH, irCode, 0, 0);

	if(!success) return 0;

	map_gap(remote, &start, &last, 0, repeat_flagp,min_remaining_gapp, max_remaining_gapp);
	
	return 1;
}

ir_code get_ir_code() {

	if(!receive) return 0;
	ir_code currentCode;

	receive->getData(&currentCode);
	ResetEvent(dataReadyEvent);

	return currentCode;
}

void wait_for_data(lirc_t timeout) {

	if(!receive) return;
	receive->waitTillDataIsReady(timeout);
}

int data_ready() {

	if(!receive) return 0;
	return receive->dataReady();
}

struct hardware hw;

void initHardwareStruct() {

	hw.decode_func	= &gnt_decode;
	hw.readdata		= NULL;
	hw.wait_for_data= &wait_for_data;
	hw.data_ready	= &data_ready;
	hw.get_ir_code	= &get_ir_code;

	hw.features		= LIRC_CAN_REC_LIRCCODE;
	hw.send_mode	= 0;
	hw.rec_mode		= LIRC_MODE_LIRCCODE;
	hw.code_length	= 32;
	hw.resolution	= 0;

	strcpy(hw.device,"hw");
	strcpy(hw.name,"Geniatech");
}