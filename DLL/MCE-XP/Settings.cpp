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
#include "MCEDefines.h"

Settings::Settings() {

	loadSettings();
}

void Settings::setTransmitterChannels(int channels) {

	if(channels<0)	channels = 0;
	if(channels>7)	channels = 7;
	
	transmitterChannels = channels;
}

int Settings::getTransmitterChannels() {

	return transmitterChannels;
}

void Settings::saveSettings() {

	winlirc_settings_set_int(L"MCEVista", L"TransmitterChannels", transmitterChannels);
}

void Settings::loadSettings() {

	transmitterChannels = winlirc_settings_get_int(L"MCEVista", L"TransmitterChannels", MCE_BLASTER_BOTH);
}