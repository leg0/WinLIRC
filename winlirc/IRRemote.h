#pragma once

#include <winlirc/PluginApi.h>

int winlirc_map_code(ir_remote* remote,
	ir_code* prep, ir_code* codep, ir_code* postp,
	int pre_bits, ir_code pre,
	int bits, ir_code code,
	int post_bits, ir_code post);

void winlirc_map_gap(ir_remote* remote,
	int64_t gap_us,
	lirc_t signal_length,
	int* repeat_flagp,
	lirc_t* min_remaining_gapp,
	lirc_t* max_remaining_gapp);

bool winlirc_decodeCommand(
	rbuf* rec_buffer,
	hardware const* hw,
	ir_remote* remotes,
	char* out,
	size_t out_size);

ir_code get_ir_code(ir_ncode* ncode, ir_code_node* node);
ir_code_node* get_next_ir_code_node(ir_ncode* ncode, ir_code_node* node);
int bit_count(ir_remote const* remote);
static inline int bits_set(ir_code data) noexcept { return winlirc_interface::bits_set(data); }
static constexpr ir_code reverse(ir_code data, int bits) { return winlirc_interface::reverse(data, bits); }
static constexpr bool is_pulse(lirc_t data) noexcept { return winlirc_interface::is_pulse(data); }
static constexpr bool is_space(lirc_t data) noexcept { return winlirc_interface::is_space(data); }
static constexpr ir_code gen_mask(int bits) noexcept { return winlirc_interface::gen_mask(bits); }
bool has_repeat(ir_remote const* remote);
void set_protocol(ir_remote* remote, int protocol);
bool is_raw(ir_remote const* remote);
bool is_space_enc(ir_remote const* remote);
bool is_space_first(ir_remote const* remote);
bool is_rc5(ir_remote const* remote);
bool is_rc6(ir_remote const* remote);
bool is_biphase(ir_remote const* remote);
bool is_rcmm(ir_remote const* remote);
bool is_goldstar(ir_remote const* remote);
bool is_grundig(ir_remote const* remote);
bool is_bo(ir_remote const* remote);
bool is_serial(ir_remote const* remote);
bool is_xmp(ir_remote const* remote);
bool is_const(ir_remote const* remote);
bool has_repeat_gap(ir_remote const* remote);
bool has_pre(ir_remote const* remote);
bool has_post(ir_remote const* remote);
bool has_header(ir_remote const* remote);
bool has_foot(ir_remote const* remote);
bool has_toggle_bit_mask(ir_remote const* remote);
bool has_ignore_mask(ir_remote const* remote);
bool has_toggle_mask(ir_remote const* remote);
lirc_t min_gap(ir_remote const* remote);
lirc_t max_gap(ir_remote const* remote);
ir_code gen_ir_code(ir_remote const* remote, ir_code pre, ir_code code, ir_code post);
bool match_ir_code(ir_remote const* remote, ir_code a, ir_code b);
bool expect(ir_remote const* remote, lirc_t delta, lirc_t exdelta);
bool expect_at_least(ir_remote const* remote, lirc_t delta, lirc_t exdelta);
bool expect_at_most(ir_remote const* remote, lirc_t delta, lirc_t exdelta);
unsigned get_freq(ir_remote const* remote);
void set_freq(ir_remote* remote, unsigned freq);
unsigned get_duty_cycle(ir_remote const* remote);
void set_duty_cycle(ir_remote* remote, unsigned duty_cycle);
