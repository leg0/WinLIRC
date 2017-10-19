#ifndef SEND_H
#define SEND_H

#include "LIRCDefines.h"
#include "winlirc_api.h"

//extern struct sbuf send_buffer;

WINLIRC_API int get_send_buffer_length(void);
WINLIRC_API lirc_t const* get_send_buffer_data(void);
WINLIRC_API void init_send_buffer(void);
WINLIRC_API int init_send(struct ir_remote *remote,struct ir_ncode *code,int repeats);

#endif