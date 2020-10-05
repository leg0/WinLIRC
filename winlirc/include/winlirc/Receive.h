#pragma once

#include "winlirc_api.h"

WINLIRC_API void init_rec_buffer();
WINLIRC_API int clear_rec_buffer(hardware const* hw);

WINLIRC_API int receive_decode(hardware const* hw, ir_remote* remote,
	ir_code* prep, ir_code* codep, ir_code* postp,
	int* repeat_flagp,
	lirc_t* min_remaining_gapp, lirc_t* max_remaining_gapp);
