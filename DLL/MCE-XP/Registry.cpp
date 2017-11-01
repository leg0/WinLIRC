#include <Windows.h>
#include <tchar.h>
#include "Registry.h"
#include <stdio.h>

BOOL RegistrySettings::hidEnabled() {

	//=============
	HKEY	key;
	LONG	result;
	DWORD	size;
	//=============

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,_T("SYSTEM\\CurrentControlSet\\Services\\HidIr\\Remotes\\745a17a0-74d3-11d0-b6fe-00a0c90f57da"),0,KEY_READ,&key);

	if(result!=ERROR_SUCCESS) {
		return TRUE;	// if we can't open HID we lie and say its enabled, since we can't change it anyway
	}

	size = sizeof(DWORD);

	result = RegQueryValueEx(key,_T("CodeSetNum0"),nullptr,nullptr,nullptr,nullptr);

	if(result!=ERROR_SUCCESS) {
		RegCloseKey(key);
		return FALSE;
	}

	result = RegQueryValueEx(key,_T("CodeSetNum0"),nullptr,nullptr,nullptr,nullptr);

	if(result!=ERROR_SUCCESS) {
		RegCloseKey(key);
		return FALSE;
	}

	result = RegQueryValueEx(key,_T("CodeSetNum0"),nullptr,nullptr,nullptr,nullptr);

	if(result!=ERROR_SUCCESS) {
		RegCloseKey(key);
		return FALSE;
	}

	result = RegQueryValueEx(key,_T("CodeSetNum0"),nullptr,nullptr,nullptr,nullptr);

	if(result!=ERROR_SUCCESS) {
		RegCloseKey(key);
		return FALSE;
	}

	RegCloseKey(key);

	return TRUE;
}

BOOL RegistrySettings::setHIDState(BOOL enabled) {

	//=============
	HKEY	key;
	LONG	result;
	//=============

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,_T("SYSTEM\\CurrentControlSet\\Services\\HidIr\\Remotes\\745a17a0-74d3-11d0-b6fe-00a0c90f57da"),0,KEY_SET_VALUE,&key);

	if(result!=ERROR_SUCCESS) {
		return FALSE;	// if we can't open HID we lie and say its enabled, since we can't change it anyway
	}

	if(enabled) {

		//=========
		DWORD value;
		//=========

		value = 1;
		RegSetValueEx(key, _T("CodeSetNum0"),0,REG_DWORD,(BYTE*)&value,sizeof(value));
		value = 2;
		RegSetValueEx(key, _T("CodeSetNum1"),0,REG_DWORD,(BYTE*)&value,sizeof(value));
		value = 3;
		RegSetValueEx(key, _T("CodeSetNum2"),0,REG_DWORD,(BYTE*)&value,sizeof(value));
		value = 4;
		RegSetValueEx(key, _T("CodeSetNum3"),0,REG_DWORD,(BYTE*)&value,sizeof(value));

	}
	else {

		RegDeleteValue(key,_T("CodeSetNum0"));
		RegDeleteValue(key,_T("CodeSetNum1"));
		RegDeleteValue(key,_T("CodeSetNum2"));
		RegDeleteValue(key,_T("CodeSetNum3"));
	}

	RegCloseKey(key);

	return TRUE;
}