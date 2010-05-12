#ifndef DECODE_H
#define DECODE_H

void init_rec_buffer(void);
bool decodeCommand(struct ir_remote *remotes, char *out);
int receive_decode(struct ir_remote *remote,
		   ir_code *prep,ir_code *codep,ir_code *postp,
		   int *repeat_flagp,
		   lirc_t *min_remaining_gapp, lirc_t *max_remaining_gapp);

#endif