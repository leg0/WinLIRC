#pragma once

#include "LIRCDefines.h"
#include "winlirc_api.h"

struct ir_remote;
struct hardware;
struct mytimeval;

WINLIRC_API int map_code(ir_remote* remote,
	ir_code* prep, ir_code* codep, ir_code* postp,
	int pre_bits, ir_code pre,
	int bits, ir_code code,
	int post_bits, ir_code post);

WINLIRC_API void map_gap(struct ir_remote* remote,
	mytimeval* start, mytimeval* last,
	lirc_t signal_length,
	int* repeat_flagp,
	lirc_t* min_remaining_gapp,
	lirc_t* max_remaining_gapp);

WINLIRC_API bool decodeCommand(hardware const* hw, ir_remote* remotes, char* out);
