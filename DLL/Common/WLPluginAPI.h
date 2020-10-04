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

#include <stdint.h>

//
// API
//

#ifdef __cplusplus
    #define WL_API_EXTERNC extern "C"
#else
    #define WL_API_EXTERNC
#endif

#if defined(winlirc_EXPORTS)
    #define WL_API WL_API_EXTERNC
#else
    #define WL_API WL_API_EXTERNC __declspec(dllexport)
#endif

static const uint32_t winlirc_plugin_api_version = 1;

typedef struct ir_remote ir_remote;
typedef struct ir_ncode ir_ncode;
typedef struct hardware hardware;
typedef uint64_t ir_code;
typedef int lirc_t;
typedef uintptr_t WLEventHandle;

#pragma pack(push, 4)
struct plugin_interface
{
	uint32_t plugin_api_version;
	int	(*init)(WLEventHandle exitEvent);
	void (*deinit)(void);
	int	(*hasGui)(void);
	void (*loadSetupGui)(void);
	int	(*sendIR)(ir_remote* remote, ir_ncode* code, int32_t repeats);
	int	(*decodeIR)(ir_remote* remotes, char* out, size_t out_size);
	int	(*setTransmitters)(uint32_t transmitterMask);
	hardware const* (*getHardware)(void);
	hardware const* hardware;
};

struct hardware
{
	uint32_t plugin_api_version;

	char device[128];
	char name[128];

	uint32_t features;
	uint32_t send_mode;
	uint32_t rec_mode;
	uint32_t code_length;
	uint32_t resolution;

	int (*decode_func)(hardware const* hw, ir_remote* remote,
		ir_code* prep, ir_code* codep, ir_code* postp,
		int* repeat_flag,
		lirc_t* min_remaining_gapp,
		lirc_t* max_remaining_gapp);

	lirc_t(*readdata)(lirc_t timeout);
	void(*wait_for_data)(lirc_t timeout);
	int(*data_ready)(void);
	ir_code(*get_ir_code)(void);
};
#pragma pack(pop)

WL_API plugin_interface const* getPluginInterface();

WL_API int	init			(WLEventHandle exitEvent);
WL_API void	deinit			();
WL_API int	hasGui			();
WL_API void	loadSetupGui	();
WL_API int	sendIR			(ir_remote* remote, ir_ncode *code, int repeats);
WL_API int	decodeIR		(ir_remote* remotes, char* out, size_t out_size);
WL_API int	setTransmitters	(unsigned int transmitterMask);
WL_API hardware* getHardware();							// optional API for IRRecord
