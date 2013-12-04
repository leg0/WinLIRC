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
 * Copyright (C) 2011 Artem Golubev
 */

#ifndef RECEIVEDATA_H
#define RECEIVEDATA_H

#include <windows.h>
#include <tchar.h>
#include <cstdio>

#include "../Common/LIRCDefines.h"

#include "BeholdRC.h"

class SendReceiveData
{
public:
	SendReceiveData();
   ~SendReceiveData();

	BOOL   init();
	void   deinit();

	int		dataReady();
	bool	waitTillDataIsReady( int maxUSecs );
	void	threadProc();

private:
	void	killThread();
	void	getCode(); // @TODO

	HANDLE	threadHandle;
	HANDLE	exitEvent;
};

#endif