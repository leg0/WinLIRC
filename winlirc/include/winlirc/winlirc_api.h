#pragma once

//
// This file contains the API that winlirc exposes to plugins to use.
//

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

#include <stdint.h>

typedef struct ir_remote ir_remote;
typedef struct ir_ncode ir_ncode;
typedef struct hardware hardware;
typedef uint64_t ir_code;
typedef int lirc_t;


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
	hardware const* hw,
	ir_remote* remotes,
	char* out,
	size_t out_size);

// Receive

WINLIRC_API void init_rec_buffer();
WINLIRC_API int clear_rec_buffer(hardware const* hw);

WINLIRC_API int receive_decode(hardware const* hw, ir_remote* remote,
	ir_code* prep, ir_code* codep, ir_code* postp,
	int* repeat_flagp,
	lirc_t* min_remaining_gapp, lirc_t* max_remaining_gapp);

// Send

WINLIRC_API int get_send_buffer_length();
WINLIRC_API lirc_t const* get_send_buffer_data();
WINLIRC_API void init_send_buffer();
WINLIRC_API int init_send(ir_remote *remote, ir_ncode *code, int repeats);
