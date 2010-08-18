#include  <Windows.h>
#include "Settings.h"

Settings::Settings() {

}

bool Settings::getPluginName(TCHAR out[]) {

	//=====================
	TCHAR path[MAX_PATH];
	int count;
	//=====================

	GetCurrentDirectory(MAX_PATH,path);

	_tcscat_s(path,_countof(path),_T("\\WinLIRC.ini"));

	count = GetPrivateProfileString(_T("WinLIRC"),_T("Plugin"),NULL,pluginName,_countof(pluginName),path);

	if(count) {
		_tcscpy_s(out,128,pluginName);
		return true;
	}

	return false;
}