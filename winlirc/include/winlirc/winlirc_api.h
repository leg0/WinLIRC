#pragma once

//
// This file contains the API that winlirc exposes to plugins to use.
//

#include <stdint.h>

#if defined(__cplusplus)
#define WINLIRC_EXTERNC extern "C"
#else
#define WINLIRC_EXTERNC
#endif

#if defined(winlirc_EXPORTS)
#define WINLIRC_API WINLIRC_EXTERNC __declspec(dllexport)
#else
#define WINLIRC_API WINLIRC_EXTERNC __declspec(dllimport)
#endif

#define RBUF_SIZE		(256)
#define WBUF_SIZE		(2048)

typedef struct ir_remote ir_remote;
typedef struct ir_ncode ir_ncode;
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
