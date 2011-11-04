#ifndef SEND_H
#define SEND_H

extern struct sbuf send_buffer;

void init_send_buffer(void);
int init_send(struct ir_remote *remote,struct ir_ncode *code,int repeats);

#endif