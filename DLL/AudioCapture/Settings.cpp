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

	_tcscpy_s(m_deviceName,32,name);
}

void Settings::getAudioDeviceName(TCHAR *out) {

	_tcscpy_s(out,32,m_deviceName);
}

void Settings::setAudioFormat(int format) {

	m_audioFormat = format;
}

int Settings::getAudioFormat() {
	
	return m_audioFormat;
}

void Settings::setChannel(bool left) {

	m_leftChannel = left;
}

bool Settings::getChannel() {
	
	return m_leftChannel;
}

void Settings::setPolarity(SP p) {

	m_polarity = p;
}

SP Settings::getPolarity() {

	return m_polarity;
}

int Settings::getNoiseValue() {

	return m_noiseValue;
}

void Settings::setNoiseValue(int n) {

	m_noiseValue = n;
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

	WritePrivateProfileString(_T("AudioInputPlugin"),_T("AudioDeviceName"), m_deviceName, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), m_audioFormat);
	WritePrivateProfileString(_T("AudioInputPlugin"),_T("AudioFormat"),	temp, currentDirectory);
	
	_sntprintf(temp, _countof(temp), _T("%i"), m_leftChannel);
	WritePrivateProfileString(_T("AudioInputPlugin"),_T("LeftChannel"),	temp, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), m_polarity);
	WritePrivateProfileString(_T("AudioInputPlugin"),_T("Polarity"),	temp, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), m_noiseValue);
	WritePrivateProfileString(_T("AudioInputPlugin"),_T("NoiseValue"),	temp, currentDirectory);
}

void Settings::loadSettings() {

	//===============================
	TCHAR currentDirectory[MAX_PATH];
	//===============================

	GetCurrentDirectory(MAX_PATH,currentDirectory);

	_tcscat(currentDirectory, _T("\\WinLIRC.ini"));

	GetPrivateProfileString(_T("AudioInputPlugin"),_T("AudioDeviceName"),NULL,m_deviceName,32,currentDirectory);

	m_audioFormat	= GetPrivateProfileInt(_T("AudioInputPlugin"),_T("AudioFormat"),1,currentDirectory);
	m_leftChannel	= (GetPrivateProfileInt(_T("AudioInputPlugin"),_T("LeftChannel"),1,currentDirectory)!=0);	//to shut the compiler up
	m_polarity		= (SP)GetPrivateProfileInt(_T("AudioInputPlugin"),_T("Polarity"),(INT)SP_AUTOMATIC,currentDirectory);	//to shut the compiler up
	m_noiseValue	= GetPrivateProfileInt(_T("AudioInputPlugin"),_T("NoiseValue"),16,currentDirectory);	//to shut the compiler up
}