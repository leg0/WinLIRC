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

#include "Settings.h"
#include <winlirc/winlirc_api.h>
#include <tchar.h>
#include <Windows.h>

Settings::Settings() {

	loadSettings();
}

void Settings::setComPort(int port) {

	comPort = port;

	if(comPort<1) comPort = 1;
}

int Settings::getComPort() {

	return comPort;
}

void Settings::saveSettings() {

	winlirc_settings_set_int(L"IRMan", L"ComPort", comPort);
}

void Settings::loadSettings() {

	comPort = winlirc_settings_get_int(L"IRMan", L"ComPort", 1);

	// make sure that comPort is in range 0..255
	if (comPort < 0 || 255 < comPort) {
		comPort = 0;
	}
}