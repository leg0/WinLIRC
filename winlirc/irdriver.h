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
 * Modifications Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#pragma once

#include "Plugin.h"
#include <string>

struct ir_remote;
struct ir_ncode;

class CIRDriver
{
public:
	CIRDriver();
	~CIRDriver();

	BOOL	loadPlugin		(std::wstring plugin);
	void	unloadPlugin	();
	BOOL	init			();
	void	deinit			();
	int		sendIR			(ir_remote* remote, ir_ncode* code, int repeats);
	int		decodeIR		(ir_remote* remote, char* out, size_t out_size);
	int		setTransmitters	(unsigned int transmitterMask);

	void	DaemonThreadProc() const;

private:

	/// Protects access to the functions imported from plug-in dll, and the
	/// DLL handle.
	mutable CCriticalSection	m_dllLock;

	//===============================
	Plugin		m_dll;
	std::wstring m_loadedPlugin;
	HANDLE m_daemonThreadEvent;
	CWinThread*	m_daemonThreadHandle;
	//===============================
};
