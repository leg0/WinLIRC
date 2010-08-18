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

#ifndef HARDWARE_H
#define HARDWARE_H

#include "LIRCDefines.h"
#include <Windows.h>

void initHardwareStruct();

//
// this hardware struct differs somewhat from the LIRC project
// but the functions we need for IR record should be there
// the rest are exported normally from the DLL
//

struct hardware
{
	char device[128];
	char name[128];

	unsigned long features;
	unsigned long send_mode;
	unsigned long rec_mode;
	unsigned long code_length;
	unsigned int resolution;

	int (*decode_func)(struct ir_remote *remote,
		ir_code *prep,ir_code *codep,ir_code *postp,
		int *repeat_flag,
		lirc_t *min_remaining_gapp,
		lirc_t *max_remaining_gapp);

	lirc_t	(*readdata)		(lirc_t timeout);
	void	(*wait_for_data)(lirc_t timeout);
	int		(*data_ready)	(void);
	ir_code (*get_ir_code)	(void);
};

#endif


