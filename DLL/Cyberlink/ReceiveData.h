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
 * Copyright (C) 2013 Jan Dubiec <jdx(at)onet(dot)pl>
 */

#pragma once

#include <winlirc/WLPluginAPI.h>
#include <chrono>
#include <vector>
#include <string>
#include <winusb.h>

class ReceiveData
{
public:
	typedef std::vector<std::wstring> StringList;

	ReceiveData();

	bool	init();
	void	deinit();

	bool	getData(lirc_t *out);
	bool	dataReady();
	bool	waitTillDataIsReady(std::chrono::microseconds maxUSecs);
	void	threadProc(int threadNumber);

private:

	void	setData(lirc_t data);
	void	findCyberLinkDevicePaths(LPGUID  pGuid );

	//==========================
	lirc_t			dataBuffer[256];
	UCHAR			bufferStart;
	UCHAR			bufferEnd;
	HANDLE			dataBufferSemaphore;
	StringList		CyberLinkDevicePaths;
	int				nDevices;
	HANDLE			deviceHandle[2];
	WINUSB_INTERFACE_HANDLE usbHandle[2];
	UCHAR			inPipeId[2];
	USHORT			inMaxPacketSize[2];
	HANDLE			threadHandle[2];
	HANDLE			exitEvent[2];
	//==========================
};
