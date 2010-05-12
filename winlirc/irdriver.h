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

#ifndef IRDRIVER_H
#define IRDRIVER_H

#include "globals.h"

#include "stdafx.h"
#include "winlirc.h"


class CIRDriver {

public:
	CIRDriver();
   ~CIRDriver();

	BOOL	loadPlugin	(CString plugin);
	void	unloadPlugin();
	BOOL	init		();
	void	deinit		();
	int		sendIR		(struct ir_remote *remote,struct ir_ncode *code, int repeats);
	int		decodeIR	(struct ir_remote *remote, char *out);

	void	DaemonThreadProc();

private:

	typedef int	 (*InitFunction)			(HANDLE);
	typedef void (*DeinitFunction)			(void);
	typedef int  (*HasGuiFunction)			(void);
	typedef void (*LoadSetupGuiFunction)	(void);
	typedef int	 (*SendFunction)			(struct ir_remote *remote,struct ir_ncode *code, int repeats);
	typedef int  (*DecodeFunction)			(struct ir_remote *remote, char *out);

	InitFunction			initFunction;
	DeinitFunction			deinitFunction;
	HasGuiFunction			hasGuiFunction;
	LoadSetupGuiFunction	loadSetupGuiFunction;
	SendFunction			sendFunction;
	DecodeFunction			decodeFunction;

	//==============================
	CString		loadedPlugin;
	HMODULE		dllFile;
	HANDLE		daemonThreadEvent;
	CWinThread	*daemonThreadHandle;
	//==============================
};

#endif
