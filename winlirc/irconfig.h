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

#pragma once

#include "globals.h"

#include "stdafx.h"
#include "irdriver.h"
#include <filesystem>

class CIRConfig
{
public:
	CIRConfig();
	~CIRConfig();

	bool readConfig	();
	bool writeINIFile();
	bool readINIFile();

	//=============================
	CString remoteConfig;
	std::filesystem::path plugin;
	BOOL	disableRepeats;
	INT		disableFirstKeyRepeats;
	INT		serverPort;
	BOOL	localConnectionsOnly;
	BOOL	showTrayIcon;
	BOOL	exitOnError;
	//=============================
};

/* Change this stuff */
extern ir_remote* global_remotes;
extern class CCriticalSection CS_global_remotes;
extern class CIRConfig config;
