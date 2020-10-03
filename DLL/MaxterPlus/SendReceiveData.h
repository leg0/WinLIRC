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

#ifndef RECEIVEDATA_H
#define RECEIVEDATA_H

#include "../Common/LIRCDefines.h"

#if !defined __drv_maxIRQL
#define __drv_maxIRQL(x)
#define __drv_when( p1, p2 )
#define __drv_freesMem( x )
#endif

extern "C" {
	#include "hid.h"
}

class SendReceiveData
{
public:
	SendReceiveData();

	bool	init();
	void	deinit();

	bool	waitTillDataIsReady(std::chrono::microseconds maxUSecs);
	void	threadProc();
	int		decodeCommand(char *out, size_t out_size);
	
private:

	BOOL	setFeatures();
	void	restoreFeatures();

	HANDLE	threadHandle;
	HANDLE	exitEvent;

	//=====================
	HID_DEVICE	device;
	BOOL		toggleBit;
	UCHAR		irCode;
	UCHAR		lastValue;
	UINT		repeats;
	//=====================
};

#endif