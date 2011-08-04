/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.9.0.
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
 * Copyright (C) 2011 Ian Curtis
 */

#ifndef XBOX360_h
#define XBOX360_h

//
// Export API
//
#define XB_API __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

XB_API int	init		(HANDLE exitEvent);
XB_API void	deinit		();
XB_API int	hasGui		();
XB_API void	loadSetupGui();
XB_API int	sendIR		(struct ir_remote *remote, struct ir_ncode *code, int repeats);
XB_API int	decodeIR	(struct ir_remote *remotes, char *out);

#ifdef __cplusplus
}
#endif

#endif