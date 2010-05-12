#ifndef EMULATION_H
#define EMULATION_H

extern struct ir_remote *emulation_data;
extern struct ir_ncode *next_code;
extern struct ir_ncode *current_code;
extern int current_index;
extern int current_rep;

void init_rec_buffer(void);
int clear_rec_buffer(void);
int receive_decode(struct ir_remote *remote,
		   ir_code *prep,ir_code *codep,ir_code *postp,
		   int *repeat_flagp,
		   lirc_t *min_remaining_gapp, lirc_t *max_remaining_gapp);
lirc_t emulation_readdata(lirc_t timeout);

#endif