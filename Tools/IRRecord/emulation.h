#pragma once

extern struct ir_remote *emulation_data;
extern std::vector<ir_ncode>::iterator next_code;
extern std::vector<ir_ncode>::iterator current_code;
extern int current_index;
extern int current_rep;

void init_rec_buffer() noexcept;
int clear_rec_buffer() noexcept;
int receive_decode(struct ir_remote *remote,
		   ir_code *prep,ir_code *codep,ir_code *postp,
		   int *repeat_flagp,
		   lirc_t *min_remaining_gapp, lirc_t *max_remaining_gapp);
lirc_t emulation_readdata(lirc_t timeout);
