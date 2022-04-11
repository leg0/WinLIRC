#include "Settings.h"
#include <winlirc/winlirc_api.h>

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
	
	winlirc_settings_set_int(L"TiraPlugin", L"ComPort", comPort);
}

void Settings::loadSettings() {

	comPort = winlirc_settings_get_int(L"TiraPlugin", L"ComPort", 0);
}