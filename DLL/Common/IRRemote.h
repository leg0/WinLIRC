#pragma once

#include "LIRCDefines.h"
#include "winlirc_api.h"

struct ir_remote;
struct hardware;

WINLIRC_API int winlirc_map_code(ir_remote* remote,
	ir_code* prep, ir_code* codep, ir_code* postp,
	int pre_bits, ir_code pre,
	int bits, ir_code code,
	int post_bits, ir_code post);

WINLIRC_API void winlirc_map_gap(ir_remote* remote,
	int64_t gap_us,
	lirc_t signal_length,
	int* repeat_flagp,
	lirc_t* min_remaining_gapp,
	lirc_t* max_remaining_gapp);

WINLIRC_API bool winlirc_decodeCommand(hardware const* hw, ir_remote* remotes, char* out, size_t out_size);
