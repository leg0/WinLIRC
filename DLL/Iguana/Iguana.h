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

#ifndef IGUANA_H
#define IGUANA_H

//
// API
//
#define IG_API __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

IG_API int	init			(HANDLE exitEvent);
IG_API void	deinit			();
IG_API int	hasGui			();
IG_API void	loadSetupGui	();
IG_API int	sendIR			(struct ir_remote *remote, struct ir_ncode *code, int repeats);
IG_API int	decodeIR		(struct ir_remote *remotes, char *out);
IG_API int	setTransmitters	(unsigned int transmitterMask);

//
// This function will be for the IR-record port, well that's the plan anyway
// It's not needed by the main app
//
IG_API struct hardware* getHardware();

#ifdef __cplusplus
}
#endif

#endif