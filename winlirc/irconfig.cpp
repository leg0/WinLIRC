/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
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
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#include "stdafx.h"
#include "irconfig.h"
#include "irdriver.h"
#include <atlbase.h>
#include "config.h"
#include "remote.h"
#include "config.h"
#include "wl_debug.h"

std::unique_ptr<ir_remote> global_remotes;
std::mutex CS_global_remotes;
CIRConfig config;

CIRConfig::CIRConfig() {

	std::lock_guard lock{ CS_global_remotes };
	
	global_remotes.reset();
	exitOnError = FALSE;
}

CIRConfig::~CIRConfig()
{
	WL_DEBUG("~CIRConfig\n");
		
	std::lock_guard lock{ CS_global_remotes };

	global_remotes.reset();
	
	WL_DEBUG("~CIRConfig done\n");
}

bool CIRConfig::readConfig() {

	//========================================
	std::lock_guard lock{ CS_global_remotes };
	FILE *file;
	//========================================

	if(remoteConfig.empty() || (file=_tfopen(remoteConfig.c_str(),_T("r")))==nullptr)	
		return false;

	global_remotes.reset();
	
	USES_CONVERSION;
	global_remotes = read_config(file,T2A(remoteConfig.c_str()));

	fclose(file);

	/* ??? bad input causes codes to be null, but no */
	/* error is returned from read_config. */
	for (auto sr = global_remotes.get(); sr != nullptr; sr = sr->next.get())
	{
		if (sr->codes.empty())
		{
			WL_DEBUG("read_config returned remote with null codes\n");
			global_remotes.reset();
			return false;
		}
	}

	if (global_remotes == nullptr)
	{
		WL_DEBUG("read_config returned null\n");
		return false;
	}

	return true;
}

bool CIRConfig::writeINIFile() {

	//=====================
	TCHAR	path[MAX_PATH];
	CString tempPath;
	TCHAR	tempIni[16];
	//=====================

	GetCurrentDirectory(MAX_PATH,path);

	tempPath += path;
	tempPath += _T("\\WinLIRC.ini");

	WritePrivateProfileString(_T("WinLIRC"),_T("RemoteConfig"),remoteConfig.c_str(),tempPath);
	WritePrivateProfileString(_T("WinLIRC"),_T("Plugin"),plugin.c_str(),tempPath);

	_sntprintf(tempIni,_countof(tempIni),_T("%i"),disableRepeats);
	WritePrivateProfileString(_T("WinLIRC"),_T("DisableKeyRepeats"),tempIni,tempPath);

	_sntprintf(tempIni,_countof(tempIni),_T("%i"),disableFirstKeyRepeats);
	WritePrivateProfileString(_T("WinLIRC"),_T("DisableFirstKeyRepeats"),tempIni,tempPath);

	_sntprintf(tempIni,_countof(tempIni),_T("%i"),localConnectionsOnly);
	WritePrivateProfileString(_T("WinLIRC"),_T("LocalConnectionsOnly"),tempIni,tempPath);

	_sntprintf(tempIni,_countof(tempIni),_T("%i"),serverPort);
	WritePrivateProfileString(_T("WinLIRC"),_T("ServerPort"),tempIni,tempPath);

	_sntprintf(tempIni,_countof(tempIni),_T("%i"),showTrayIcon);
	WritePrivateProfileString(_T("WinLIRC"),_T("ShowTrayIcon"),tempIni,tempPath);

	return true;
}

bool CIRConfig::readINIFile() {

	//=================================
	TCHAR	path[MAX_PATH];
	CString tempPath;
	TCHAR   remoteConfigName[MAX_PATH];
	TCHAR	pluginName[MAX_PATH];
	//=================================

	GetCurrentDirectory(MAX_PATH,path);

	tempPath += path;
	tempPath += _T("\\WinLIRC.ini");

	GetPrivateProfileString(_T("WinLIRC"),_T("RemoteConfig"),nullptr,remoteConfigName,_countof(remoteConfigName),tempPath);
	GetPrivateProfileString(_T("WinLIRC"),_T("Plugin"),nullptr,pluginName,_countof(pluginName),tempPath);

	disableRepeats			= GetPrivateProfileInt(_T("WinLIRC"),_T("DisableKeyRepeats"),FALSE,tempPath);
	disableFirstKeyRepeats	= GetPrivateProfileInt(_T("WinLIRC"),_T("DisableFirstKeyRepeats"),FALSE,tempPath);
	localConnectionsOnly	= GetPrivateProfileInt(_T("WinLIRC"),_T("LocalConnectionsOnly"),TRUE,tempPath);
	serverPort				= GetPrivateProfileInt(_T("WinLIRC"),_T("ServerPort"),8765,tempPath);
	showTrayIcon			= GetPrivateProfileInt(_T("WinLIRC"),_T("ShowTrayIcon"),TRUE,tempPath);

	remoteConfig			= remoteConfigName;
	plugin					= pluginName;
	
	return true;
}
