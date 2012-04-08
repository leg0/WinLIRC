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
#include "hardware.h"
#include "Decode.h"

struct hardware hw;

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

void initHardwareStruct() {

	hw.decode_func	= &tevii_decode;
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
	strcpy(hw.name,"TbsNxp");
}
