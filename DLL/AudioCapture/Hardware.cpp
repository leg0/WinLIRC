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
#include "../Common/LIRCDefines.h"
#include "../Common/Receive.h"
#include "../Common/WLPluginAPI.h"

struct hardware hw;

lirc_t readdata(lirc_t timeout) {

	//==========
	lirc_t data;
	//==========

	if(!analyseAudio) return 0;

	waitTillDataIsReady(std::chrono::microseconds{ timeout });

	if(analyseAudio->getData((UINT*)&data)) {
		
		return data;
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

void initHardwareStruct() {

	hw.decode_func	= &receive_decode;
	hw.readdata		= &readdata;
	hw.wait_for_data= &wait_for_data;
	hw.data_ready	= &data_ready;
	hw.get_ir_code	= nullptr;

	hw.features		= LIRC_CAN_REC_MODE2;
	hw.send_mode	= 0;
	hw.rec_mode		= LIRC_MODE_MODE2;
	hw.code_length	= 0;
	hw.resolution	= 0;

	strcpy(hw.device,"hw");
	strcpy(hw.name,"audio");
}