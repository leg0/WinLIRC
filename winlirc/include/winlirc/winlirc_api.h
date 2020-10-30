#pragma once

//
// This file contains the API that winlirc exposes to plugins to use.
//

#include <intrin.h>
#include <stdint.h>

#if defined(__cplusplus)
	#define WINLIRC_EXTERNC extern "C"
	#define WINLIRC_CONSTEXPR constexpr
	#define WINLIRC_NOEXCEPT noexcept
#else
	#define WINLIRC_EXTERNC
	#define WINLIRC_CONSTEXPR
	#define WINLIRC_NOEXCEPT
#endif

#if defined(winlirc_EXPORTS)
	#define WINLIRC_API WINLIRC_EXTERNC __declspec(dllexport)
#else
	#define WINLIRC_API WINLIRC_EXTERNC __declspec(dllimport)
#endif

#define PULSE_BIT		(0x01000000)
#define PULSE_MASK		(0x00FFFFFF)
#define RBUF_SIZE		(256)
#define WBUF_SIZE		(2048)
#define REC_SYNC		(8)
#define DEFAULT_FREQ	(38000)

typedef struct ir_remote ir_remote;
typedef struct ir_ncode ir_ncode;
typedef struct ir_code_node ir_code_node;
typedef struct hardware hardware;
typedef uint64_t ir_code;
typedef int lirc_t;

struct rbuf
{
	lirc_t data[RBUF_SIZE];
	ir_code decoded;
	int rptr;
	int wptr;
	int too_long;
	int is_biphase;
	lirc_t pendingp;
	lirc_t pendings;
	lirc_t sum;
};

struct sbuf
{
	lirc_t *data;
	lirc_t _data[WBUF_SIZE];
	int wptr;
	int too_long;
	int is_biphase;
	lirc_t pendingp;
	lirc_t pendings;
	lirc_t sum;
};

// IR Remote

WINLIRC_API int winlirc_map_code(ir_remote* remote,
	ir_code* prep, ir_code* codep, ir_code* postp,
	int pre_bits, ir_code pre,
	int bits, ir_code code,
	int post_bits, ir_code post);

WINLIRC_API void winlirc_map_gap(ir_remote* remote,
	int64_t gap_us,
	lirc_t signal_length,
	int* repeat_flagp,
	lirc_t* min_remaining_gapp,
	lirc_t* max_remaining_gapp);

WINLIRC_API bool winlirc_decodeCommand(
	rbuf* rec_buffer,
	hardware const* hw,
	ir_remote* remotes,
	char* out,
	size_t out_size);

WINLIRC_API ir_code get_ir_code(ir_ncode* ncode, ir_code_node* node);
WINLIRC_API ir_code_node* get_next_ir_code_node(ir_ncode* ncode, ir_code_node* node);
WINLIRC_API int bit_count(ir_remote const* remote);
inline int bits_set(ir_code data) WINLIRC_NOEXCEPT
{
#if defined(_M_X64)
	return __popcnt64(data);
#else
	return __popcnt(data >> 32) + __popcnt(data & 0xFFFF'FFFF);
#endif
}
WINLIRC_CONSTEXPR inline ir_code reverse(ir_code data, int bits) WINLIRC_NOEXCEPT
{
	data = data << (64 - bits);
	ir_code res = 0;
	for (int i = 0; i < bits; ++i)
	{
		res |= ((data >> (63 - i)) & 1) << i;
	}
	return res;
}
WINLIRC_CONSTEXPR inline bool is_pulse(lirc_t data) WINLIRC_NOEXCEPT { return (data & PULSE_BIT) == PULSE_BIT; }
WINLIRC_CONSTEXPR inline bool is_space(lirc_t data) WINLIRC_NOEXCEPT { return !is_pulse(data); }
WINLIRC_API int has_repeat(ir_remote const* remote);
WINLIRC_API void set_protocol(ir_remote* remote, int protocol);
WINLIRC_API int is_raw(ir_remote const* remote);
WINLIRC_API int is_space_enc(ir_remote const* remote);
WINLIRC_API int is_space_first(ir_remote const* remote);
WINLIRC_API int is_rc5(ir_remote const* remote);
WINLIRC_API int is_rc6(ir_remote const* remote);
WINLIRC_API int is_biphase(ir_remote const* remote);
WINLIRC_API int is_rcmm(ir_remote const* remote);
WINLIRC_API int is_goldstar(ir_remote const* remote);
WINLIRC_API int is_grundig(ir_remote const* remote);
WINLIRC_API int is_bo(ir_remote const* remote);
WINLIRC_API int is_serial(ir_remote const* remote);
WINLIRC_API int is_xmp(ir_remote const* remote);
WINLIRC_API int is_const(ir_remote const* remote);
WINLIRC_API int has_repeat_gap(ir_remote const* remote);
WINLIRC_API int has_pre(ir_remote const* remote);
WINLIRC_API int has_post(ir_remote const* remote);
WINLIRC_API int has_header(ir_remote const* remote);
WINLIRC_API int has_foot(ir_remote const* remote);
WINLIRC_API int has_toggle_bit_mask(ir_remote const* remote);
WINLIRC_API int has_ignore_mask(ir_remote const* remote);
WINLIRC_API int has_toggle_mask(ir_remote const* remote);
WINLIRC_API lirc_t min_gap(ir_remote const* remote);
WINLIRC_API lirc_t max_gap(ir_remote const* remote);
WINLIRC_CONSTEXPR inline ir_code gen_mask(int bits) WINLIRC_NOEXCEPT { return ~0ULL & (1ULL << bits) - 1; }
WINLIRC_API ir_code gen_ir_code(ir_remote const* remote, ir_code pre, ir_code code, ir_code post);
WINLIRC_API int match_ir_code(ir_remote const* remote, ir_code a, ir_code b);
WINLIRC_API int expect(ir_remote const* remote, lirc_t delta, lirc_t exdelta);
WINLIRC_API int expect_at_least(ir_remote const* remote, lirc_t delta, lirc_t exdelta);
WINLIRC_API int expect_at_most(ir_remote const* remote, lirc_t delta, lirc_t exdelta);
WINLIRC_API unsigned get_freq(ir_remote const* remote);
WINLIRC_API void set_freq(ir_remote* remote, unsigned freq);
WINLIRC_API unsigned get_duty_cycle(ir_remote const* remote);
WINLIRC_API void set_duty_cycle(ir_remote* remote, unsigned duty_cycle);

// Receive

WINLIRC_API void winlirc_init_rec_buffer(rbuf* rec_buffer);
WINLIRC_API int winlirc_clear_rec_buffer(rbuf* rec_buffer, hardware const* hw);
WINLIRC_API int winlirc_receive_decode(rbuf* rec_buffer,
	hardware const* hw, ir_remote* remote,
	ir_code* prep, ir_code* codep, ir_code* postp,
	int* repeat_flagp,
	lirc_t* min_remaining_gapp, lirc_t* max_remaining_gapp);

// Send

WINLIRC_API int winlirc_get_send_buffer_length(sbuf const* send_buffer);
WINLIRC_API lirc_t const* winlirc_get_send_buffer_data(sbuf const* send_buffer);
WINLIRC_API void winlirc_init_send_buffer(sbuf* send_buffer);
WINLIRC_API int winlirc_init_send(sbuf* send_buffer, ir_remote *remote, ir_ncode *code, int repeats);
