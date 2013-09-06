#include "Settings.h"
#include <tchar.h>
#include <Windows.h>
#include <stdio.h>

Settings::Settings() {

	loadSettings();
}

int Settings::getComPort() {

	return comPort;
}

void Settings::setComPort(int port) {

	comPort = port;
}


void Settings::saveSettings() {

	//=================================
	TCHAR	currentDirectory[MAX_PATH];
	TCHAR	temp[8];
	FILE	*file;
	errno_t	success;
	//=================================

	GetCurrentDirectory(MAX_PATH,currentDirectory);
	_tcscat_s(currentDirectory, _T("\\WinLIRC.ini"));

	//
	// if our ini files doesn't exist try and create it
	//
	success = _tfopen_s(&file,currentDirectory,_T("r"));

	if(success!=0) {
		success = _tfopen_s(&file,currentDirectory,_T("w"));
		if(success==0) fclose(file);
	}
	else {
		fclose(file);
	}
	
	_sntprintf_s(temp, _countof(temp), _TRUNCATE, _T("%i"), comPort);
	WritePrivateProfileString(_T("TiraPlugin"),_T("ComPort"),temp, currentDirectory);
}

void Settings::loadSettings() {

	//===============================
	TCHAR currentDirectory[MAX_PATH];
	//===============================

	GetCurrentDirectory(MAX_PATH,currentDirectory);

	_tcscat_s(currentDirectory, _T("\\WinLIRC.ini"));

	comPort	= GetPrivateProfileInt(_T("TiraPlugin"),_T("ComPort"),0,currentDirectory);

}