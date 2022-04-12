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

#pragma once

#include "Serial.h"
#include <winlirc/WLPluginAPI.h>
#include <chrono>

class SendReceiveData
{
public:
	SendReceiveData() = default;

	bool	init();
	void	deinit();

	bool	getData(lirc_t *out);
	bool	dataReady();
	bool	waitTillDataIsReady(std::chrono::microseconds maxUSecs);
	void	threadProc();
	int		send(struct ir_remote *remote, struct ir_ncode *code, int repeats);
	
private:

	void	setData(lirc_t data);
	void	receiveLoop();
	UCHAR	calcPR2(int frequency) const;

	//==========================
	lirc_t		dataBuffer[256];
	UCHAR		bufferStart{ 0 };
	UCHAR		bufferEnd{ 0 };
	HANDLE		threadHandle{ nullptr };
	CSerial		serial;
	HANDLE		exitEvent{ nullptr };
	HANDLE		overlappedEvent{ nullptr };
	OVERLAPPED	overlapped{ 0 };
	//==========================
};
