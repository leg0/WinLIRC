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
#include "globals.h"
#include "config.h"

CIRConfig::CIRConfig() {

	CSingleLock lock(&CS_global_remotes,TRUE);
	
	global_remotes=NULL;
	exitOnError = FALSE;
}

CIRConfig::~CIRConfig()
{
	WL_DEBUG("~CIRConfig\n");
		
	CSingleLock lock(&CS_global_remotes,TRUE);

	if(global_remotes!=NULL) {
		free_config(global_remotes);
		global_remotes = NULL;
	}
	
	WL_DEBUG("~CIRConfig done\n");
}

bool CIRConfig::readConfig() {

	//========================================
	CSingleLock lock(&CS_global_remotes,TRUE);
	FILE *file;
	//========================================

	if(remoteConfig=="" || (file=fopen(remoteConfig,"r"))==NULL)	
		return false;

	if(global_remotes!=NULL) {
		free_config(global_remotes);
		global_remotes = NULL;
	}
	
	global_remotes = read_config(file,remoteConfig);

	fclose(file);

	if(global_remotes==(struct ir_remote *)-1)
	{
		global_remotes=NULL;
		WL_DEBUG("read_config returned -1\n");
		return false;
	}

	/* ??? bad input causes codes to be null, but no */
	/* error is returned from read_config. */
	struct ir_remote *sr;
	for(sr=global_remotes;sr!=NULL;sr=sr->next)
	{
		if(sr->codes==NULL)
		{
			WL_DEBUG("read_config returned remote with null codes\n");
			free_config(global_remotes);
			global_remotes = NULL;

			return false;
		}
	}

	if(global_remotes==NULL)
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
	TCHAR	tempIni[4];
	//=====================

	GetCurrentDirectory(MAX_PATH,path);

	tempPath += path;
	tempPath += _T("\\WinLIRC.ini");

	WritePrivateProfileString(_T("WinLIRC"),_T("RemoteConfig"),remoteConfig,tempPath);
	WritePrivateProfileString(_T("WinLIRC"),_T("Plugin"),plugin,tempPath);

	_sntprintf(tempIni,_countof(tempIni),_T("%i"),disableRepeats);
	WritePrivateProfileString(_T("WinLIRC"),_T("DisableKeyRepeats"),tempIni,tempPath);

	_sntprintf(tempIni,_countof(tempIni),_T("%i"),disableFirstKeyRepeats);
	WritePrivateProfileString(_T("WinLIRC"),_T("DisableFirstKeyRepeats"),tempIni,tempPath);

	_sntprintf(tempIni,_countof(tempIni),_T("%i"),localConnectionsOnly);
	WritePrivateProfileString(_T("WinLIRC"),_T("LocalConnectionsOnly"),tempIni,tempPath);

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

	GetPrivateProfileString(_T("WinLIRC"),_T("RemoteConfig"),NULL,remoteConfigName,_countof(remoteConfigName),tempPath);
	GetPrivateProfileString(_T("WinLIRC"),_T("Plugin"),NULL,pluginName,_countof(pluginName),tempPath);

	disableRepeats			= GetPrivateProfileInt(_T("WinLIRC"),_T("DisableKeyRepeats"),FALSE,tempPath);
	disableFirstKeyRepeats	= GetPrivateProfileInt(_T("WinLIRC"),_T("DisableFirstKeyRepeats"),FALSE,tempPath);
	localConnectionsOnly	= GetPrivateProfileInt(_T("WinLIRC"),_T("LocalConnectionsOnly"),TRUE,tempPath);
	showTrayIcon			= GetPrivateProfileInt(_T("WinLIRC"),_T("ShowTrayIcon"),TRUE,tempPath);

	remoteConfig			= remoteConfigName;
	plugin					= pluginName;
	
	return true;
}
