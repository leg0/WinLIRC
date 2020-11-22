#pragma once
#include <winlirc/PluginApi.h>

void winlirc_init_rec_buffer(rbuf* rec_buffer);
int winlirc_clear_rec_buffer(rbuf* rec_buffer, hardware const* hw);
int winlirc_receive_decode(rbuf* rec_buffer,
	hardware const* hw, ir_remote* remote,
	ir_code* prep, ir_code* codep, ir_code* postp,
	int* repeat_flagp,
	lirc_t* min_remaining_gapp, lirc_t* max_remaining_gapp);
