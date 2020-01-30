/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
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
 * Copyright (C) 2010 Ian Curtis
 */

#pragma once

//
// API
//
#define WL_API __declspec(dllexport)

typedef struct ir_remote ir_remote;
typedef struct ir_ncode ir_ncode;
typedef struct hardware hardware;

#ifdef __cplusplus
extern "C" {
#endif

WL_API int	init			(HANDLE exitEvent);
WL_API void	deinit			();
WL_API int	hasGui			();
WL_API void	loadSetupGui	();
WL_API int	sendIR			(ir_remote *remote, ir_ncode *code, int repeats);
WL_API int	decodeIR		(ir_remote *remotes, char *out);
WL_API int	setTransmitters	(unsigned int transmitterMask);

WL_API struct hardware* getHardware();							// optional API for IRRecord

#ifdef __cplusplus
}
#endif
