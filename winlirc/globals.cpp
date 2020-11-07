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
 */

#include "stdafx.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "irconfig.h"

/* Debugging stuff */

void winlirc_debug(const char *file, int line, char *format, ...)
{

	va_list args;
	va_start(args,format);

	CStringA s;
	s.Format("(%i): ",line);
	CStringA t;
	t.FormatV(format,args);

	auto const hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	static std::mutex debugOutputMutex;
	std::lock_guard g{ debugOutputMutex };
	WriteConsoleA(hStdOut, file, strlen(file), nullptr, nullptr);
	WriteConsoleA(hStdOut, static_cast<char const*>(s), s.GetLength(), nullptr, nullptr);
	WriteConsoleA(hStdOut, static_cast<char const*>(t), t.GetLength(), nullptr, nullptr);
	WriteConsoleA(hStdOut, "\n", 1, nullptr, nullptr);
}

/* End of Debugging stuff */

ir_remote* global_remotes = nullptr;

std::mutex CS_global_remotes;

CIRConfig config;
