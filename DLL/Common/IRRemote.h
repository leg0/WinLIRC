#ifndef _IRREMOTE_H_
#define _IRREMOTE_H_

int map_code(struct ir_remote *remote,
	     ir_code *prep,ir_code *codep,ir_code *postp,
	     int pre_bits,ir_code pre,
	     int bits,ir_code code,
	     int post_bits,ir_code post);

void map_gap(struct ir_remote *remote,
	     struct mytimeval *start, struct mytimeval *last,
	     lirc_t signal_length,
	     int *repeat_flagp,
	     lirc_t *min_remaining_gapp,
	     lirc_t *max_remaining_gapp);

bool decodeCommand(struct hardware const* hw, struct ir_remote *remotes, char *out);

#endif