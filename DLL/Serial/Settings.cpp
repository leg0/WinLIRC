#include "Settings.h"
#include <stdio.h>

Settings::Settings() {

	loadSettings();
}

void Settings::saveSettings() {

	//===============================
	TCHAR currentDirectory[MAX_PATH];
	TCHAR temp[32];
	FILE  *file;	
	//===============================

	GetCurrentDirectory(MAX_PATH,currentDirectory);
	_tcscat(currentDirectory, _T("\\plugins\\WinLIRC.ini"));

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

	WritePrivateProfileString(_T("SerialDevice"),_T("Port"), port, currentDirectory);
	WritePrivateProfileString(_T("SerialDevice"),_T("Speed"), speed, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), animax);
	WritePrivateProfileString(_T("SerialDevice"),_T("Animax"),	temp, currentDirectory);
	
	_sntprintf(temp, _countof(temp), _T("%i"), deviceType);
	WritePrivateProfileString(_T("SerialDevice"),_T("DeviceType"),	temp, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), virtualPulse);
	WritePrivateProfileString(_T("SerialDevice"),_T("VirtualPulse"),	temp, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), sense);
	WritePrivateProfileString(_T("SerialDevice"),_T("Sense"),	temp, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), transmitterType);
	WritePrivateProfileString(_T("SerialDevice"),_T("TransmitterType"),	temp, currentDirectory);
}

void Settings::loadSettings() {

	//===============================
	TCHAR currentDirectory[MAX_PATH];
	TCHAR temp[64];
	//===============================

	GetCurrentDirectory(MAX_PATH,currentDirectory);

	_tcscat(currentDirectory, _T("\\plugins\\WinLIRC.ini"));

	GetPrivateProfileString(_T("SerialDevice"),_T("Port"),_T("COM1"),temp,_countof(temp),currentDirectory);
	port = temp;

	GetPrivateProfileString(_T("SerialDevice"),_T("Speed"),_T("115200"),temp,_countof(temp),currentDirectory);
	speed = temp;

	animax			= GetPrivateProfileInt(_T("SerialDevice"),_T("Animax"),0,currentDirectory);
	deviceType		= GetPrivateProfileInt(_T("SerialDevice"),_T("DeviceType"),1,currentDirectory);
	virtualPulse	= GetPrivateProfileInt(_T("SerialDevice"),_T("VirtualPulse"),300,currentDirectory);
	sense			= GetPrivateProfileInt(_T("SerialDevice"),_T("Sense"),-1,currentDirectory);
	transmitterType	= GetPrivateProfileInt(_T("SerialDevice"),_T("TransmitterType"),0,currentDirectory);
}