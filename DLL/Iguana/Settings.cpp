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
#include <tchar.h>
#include <Windows.h>
#include <stdio.h>

Settings::Settings() {

	loadSettings();
}

void Settings::setDeviceNumber(int deviceNumber) {

	devNumber = deviceNumber;
}

int Settings::getDeviceNumber() {

	return devNumber;
}

void Settings::setTransmitterChannels(int channels) {

	if(channels<0)	channels = 0;
	if(channels>15) channels = 15;
	
	transmitterChannels = channels;
}

int Settings::getTransmitterChannels() {

	return transmitterChannels;
}

void Settings::saveSettings() {

	//===============================
	TCHAR currentDirectory[MAX_PATH];
	TCHAR temp[8];
	FILE  *file;	
	//===============================

	GetCurrentDirectory(MAX_PATH,currentDirectory);
	_tcscat(currentDirectory, _T("\\WinLIRC.ini"));

	//
	// if our ini files doesn't exist try and create it
	//
	file = _tfopen(currentDirectory,_T("r"));

	if(!file) {
		file = _tfopen(currentDirectory,_T("w"));
		if(file) fclose(file);
	}
	else {
		fclose(file);
	}
	
	_sntprintf(temp, _countof(temp), _T("%i"), devNumber);
	WritePrivateProfileString(_T("IguanaPlugin"),_T("DeviceNumber"),temp, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), transmitterChannels);
	WritePrivateProfileString(_T("IguanaPlugin"),_T("TransmitterChannels"),temp, currentDirectory);
}

void Settings::loadSettings() {

	//===============================
	TCHAR currentDirectory[MAX_PATH];
	//===============================

	GetCurrentDirectory(MAX_PATH,currentDirectory);

	_tcscat(currentDirectory, _T("\\WinLIRC.ini"));

	devNumber = GetPrivateProfileInt(_T("IguanaPlugin"),_T("DeviceNumber"),0,currentDirectory);
	transmitterChannels = GetPrivateProfileInt(_T("IguanaPlugin"),_T("TransmitterChannels"),0,currentDirectory);

}