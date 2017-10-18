#ifndef _RECEIVE_H_
#define _RECEIVE_H_

void init_rec_buffer(void);
int clear_rec_buffer(struct hardware const* hw);

int receive_decode(struct hardware const* hw, struct ir_remote *remote,
		   ir_code *prep,ir_code *codep,ir_code *postp,
		   int *repeat_flagp,
		   lirc_t *min_remaining_gapp, lirc_t *max_remaining_gapp);

#endif