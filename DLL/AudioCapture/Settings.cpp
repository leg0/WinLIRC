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

void Settings::saveSettings() {

	//===============================
	TCHAR currentDirectory[MAX_PATH];
	TCHAR temp[8];
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

	WritePrivateProfileString(_T("AudioInputPlugin"),_T("AudioDeviceName"), deviceName, currentDirectory);

	_sntprintf(temp, _countof(temp), _T("%i"), audioFormat);
	WritePrivateProfileString(_T("AudioInputPlugin"),_T("AudioFormat"),	temp, currentDirectory);
	
	_sntprintf(temp, _countof(temp), _T("%i"), leftChannel);
	WritePrivateProfileString(_T("AudioInputPlugin"),_T("LeftChannel"),	temp, currentDirectory);
}

void Settings::loadSettings() {

	//===============================
	TCHAR currentDirectory[MAX_PATH];
	//===============================

	GetCurrentDirectory(MAX_PATH,currentDirectory);

	_tcscat(currentDirectory, _T("\\plugins\\WinLIRC.ini"));

	GetPrivateProfileString(_T("AudioInputPlugin"),_T("AudioDeviceName"),NULL,deviceName,32,currentDirectory);

	audioFormat	= GetPrivateProfileInt(_T("AudioInputPlugin"),_T("AudioFormat"),1,currentDirectory);
	leftChannel	= (GetPrivateProfileInt(_T("AudioInputPlugin"),_T("LeftChannel"),1,currentDirectory)!=0);	//to shut the compiler up
}