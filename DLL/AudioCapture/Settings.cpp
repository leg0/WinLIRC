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

void Settings::setAudioDeviceName(TCHAR *name) {

	_tcscpy_s(deviceName,32,name);
}

void Settings::getAudioDeviceName(TCHAR *out) {

	_tcscpy_s(out,32,deviceName);
}

void Settings::setAudioFormat(int format) {

	audioFormat = format;
}

int Settings::getAudioFormat() {
	
	return audioFormat;
}

void Settings::setChannel(bool left) {

	leftChannel = left;
}

bool Settings::getChannel() {
	
	return leftChannel;
}

bool Settings::getInverted() {

	return inverted;
}

void Settings::setInverted(bool i) {

	inverted = i;
}

int Settings::getNoiseValue() {

	return noiseValue;
}

void Settings::setNoiseValue(int n) {

	noiseValue = n;
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

	WritePrivateProfileString(_T("AudioInputPlugin"),_T("AudioDeviceName"), deviceName, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), audioFormat);
	WritePrivateProfileString(_T("AudioInputPlugin"),_T("AudioFormat"),	temp, currentDirectory);
	
	_sntprintf(temp, _countof(temp), _T("%i"), leftChannel);
	WritePrivateProfileString(_T("AudioInputPlugin"),_T("LeftChannel"),	temp, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), inverted);
	WritePrivateProfileString(_T("AudioInputPlugin"),_T("Inverted"),	temp, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), noiseValue);
	WritePrivateProfileString(_T("AudioInputPlugin"),_T("NoiseValue"),	temp, currentDirectory);
}

void Settings::loadSettings() {

	//===============================
	TCHAR currentDirectory[MAX_PATH];
	//===============================

	GetCurrentDirectory(MAX_PATH,currentDirectory);

	_tcscat(currentDirectory, _T("\\WinLIRC.ini"));

	GetPrivateProfileString(_T("AudioInputPlugin"),_T("AudioDeviceName"),NULL,deviceName,32,currentDirectory);

	audioFormat	= GetPrivateProfileInt(_T("AudioInputPlugin"),_T("AudioFormat"),1,currentDirectory);
	leftChannel	= (GetPrivateProfileInt(_T("AudioInputPlugin"),_T("LeftChannel"),1,currentDirectory)!=0);	//to shut the compiler up
	inverted = (GetPrivateProfileInt(_T("AudioInputPlugin"),_T("Inverted"),0,currentDirectory)!=0);	//to shut the compiler up
	noiseValue = GetPrivateProfileInt(_T("AudioInputPlugin"),_T("NoiseValue"),16,currentDirectory);	//to shut the compiler up
}