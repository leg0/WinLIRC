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
#include "TechnotrendPlugin.h"
#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>

lirc_t readData(plugin_interface* self, lirc_t timeout)
{
	auto receive = static_cast<TechnotrendPlugin*>(self);
	if(!receive)
		return 0;

	receive->waitTillDataIsReady(timeout);

	lirc_t data = 0;
	receive->getData(&data);

	return data;
}

void wait_for_data(plugin_interface* self, lirc_t timeout)
{
	auto receive = static_cast<TechnotrendPlugin*>(self);
	if(!receive)
		return;

	receive->waitTillDataIsReady(timeout);
}

int data_ready(plugin_interface* self)
{
	auto receive = static_cast<TechnotrendPlugin*>(self);
	return receive && receive->dataReady();
}

rbuf rec_buffer;

extern hardware const technotrend_hw = {
	.device        = "hw",
	.name          = "TechnoTrend USB Infrared Receiver",
	.features      = LIRC_CAN_REC_MODE2,
	.send_mode     = 0,
	.rec_mode      = LIRC_MODE_MODE2,
	.code_length   = 0,
	.resolution    = 0,
	.decode_func   = &winlirc_receive_decode,
	.readdata      = &readData,
	.wait_for_data = &wait_for_data,
	.data_ready    = &data_ready,
	.get_ir_code   = nullptr,
};
