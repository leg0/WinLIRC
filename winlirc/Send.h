#pragma once

#include <winlirc/PluginAPI.h>

int winlirc_get_send_buffer_length(sbuf const* send_buffer);
lirc_t const* winlirc_get_send_buffer_data(sbuf const* send_buffer);
void winlirc_init_send_buffer(sbuf* send_buffer);
int winlirc_init_send(sbuf* send_buffer, ir_remote* remote, ir_ncode* code, int repeats);
