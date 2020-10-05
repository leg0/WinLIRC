#pragma once

#include "winlirc_api.h"

WINLIRC_API int get_send_buffer_length();
WINLIRC_API lirc_t const* get_send_buffer_data();
WINLIRC_API void init_send_buffer();
WINLIRC_API int init_send(ir_remote *remote, ir_ncode *code, int repeats);
