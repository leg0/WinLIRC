#pragma once

#include <stdint.h>

//
// This file contains the API that winlirc exposes to plugins to use.
//

#include <intrin.h>

#if defined(__cplusplus)
#define WINLIRC_EXTERNC extern "C"
#define WINLIRC_CONSTEXPR constexpr
#define WINLIRC_NOEXCEPT noexcept
#else
#define WINLIRC_EXTERNC
#define WINLIRC_CONSTEXPR
#define WINLIRC_NOEXCEPT
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
	lirc_t* data;
	lirc_t _data[WBUF_SIZE];
	int wptr;
	int too_long;
	int is_biphase;
	lirc_t pendingp;
	lirc_t pendings;
	lirc_t sum;
};

struct winlirc_interface
{
	int (*map_code)(ir_remote* remote,
		ir_code* prep, ir_code* codep, ir_code* postp,
		int pre_bits, ir_code pre,
		int bits, ir_code code,
		int post_bits, ir_code post);

	void (*map_gap)(ir_remote* remote,
		int64_t gap_us,
		lirc_t signal_length,
		int* repeat_flagp,
		lirc_t* min_remaining_gapp,
		lirc_t* max_remaining_gapp);

	bool (*decodeCommand)(
		rbuf* rec_buffer,
		hardware const* hw,
		ir_remote* remotes,
		char* out,
		size_t out_size);

	ir_code(*get_ir_code)(ir_ncode* ncode, ir_code_node* node);
	ir_code_node* (*get_next_ir_code_node)(ir_ncode* ncode, ir_code_node* node);
	int (*bit_count)(ir_remote const* remote);
	static inline int bits_set(ir_code data) WINLIRC_NOEXCEPT
	{
#if defined(_M_X64)
		return __popcnt64(data);
#else
		return __popcnt(data >> 32) + __popcnt(data & 0xFFFF'FFFF);
#endif
	}
	static WINLIRC_CONSTEXPR inline ir_code reverse(ir_code data, int bits) WINLIRC_NOEXCEPT
	{
		data = data << (64 - bits);
		ir_code res = 0;
		for (int i = 0; i < bits; ++i)
		{
			res |= ((data >> (63 - i)) & 1) << i;
		}
		return res;
	}
	static WINLIRC_CONSTEXPR inline bool is_pulse(lirc_t data) WINLIRC_NOEXCEPT { return (data & PULSE_BIT) == PULSE_BIT; }
	static WINLIRC_CONSTEXPR inline bool is_space(lirc_t data) WINLIRC_NOEXCEPT { return !is_pulse(data); }
	bool (*has_repeat)(ir_remote const* remote);
	void (*set_protocol)(ir_remote* remote, int protocol);
	bool (*is_raw)(ir_remote const* remote);
	bool (*is_space_enc)(ir_remote const* remote);
	bool (*is_space_first)(ir_remote const* remote);
	bool (*is_rc5)(ir_remote const* remote);
	bool (*is_rc6)(ir_remote const* remote);
	bool (*is_biphase)(ir_remote const* remote);
	bool (*is_rcmm)(ir_remote const* remote);
	bool (*is_goldstar)(ir_remote const* remote);
	bool (*is_grundig)(ir_remote const* remote);
	bool (*is_bo)(ir_remote const* remote);
	bool (*is_serial)(ir_remote const* remote);
	bool (*is_xmp)(ir_remote const* remote);
	bool (*is_const)(ir_remote const* remote);
	bool (*has_repeat_gap)(ir_remote const* remote);
	bool (*has_pre)(ir_remote const* remote);
	bool (*has_post)(ir_remote const* remote);
	bool (*has_header)(ir_remote const* remote);
	bool (*has_foot)(ir_remote const* remote);
	bool (*has_toggle_bit_mask)(ir_remote const* remote);
	bool (*has_ignore_mask)(ir_remote const* remote);
	bool (*has_toggle_mask)(ir_remote const* remote);
	lirc_t(*min_gap)(ir_remote const* remote);
	lirc_t(*max_gap)(ir_remote const* remote);
	static WINLIRC_CONSTEXPR inline ir_code gen_mask(int bits) WINLIRC_NOEXCEPT { return ~0ULL & (1ULL << bits) - 1; }
	ir_code(*gen_ir_code)(ir_remote const* remote, ir_code pre, ir_code code, ir_code post);
	bool (*match_ir_code)(ir_remote const* remote, ir_code a, ir_code b);
	bool (*expect)(ir_remote const* remote, lirc_t delta, lirc_t exdelta);
	bool (*expect_at_least)(ir_remote const* remote, lirc_t delta, lirc_t exdelta);
	bool (*expect_at_most)(ir_remote const* remote, lirc_t delta, lirc_t exdelta);
	unsigned (*get_freq)(ir_remote const* remote);
	void (*set_freq)(ir_remote* remote, unsigned freq);
	unsigned (*get_duty_cycle)(ir_remote const* remote);
	void (*set_duty_cycle)(ir_remote* remote, unsigned duty_cycle);

	// Receive

	void (*init_rec_buffer)(rbuf* rec_buffer);
	int (*clear_rec_buffer)(rbuf* rec_buffer, hardware const* hw);
	int (*receive_decode)(rbuf* rec_buffer,
		hardware const* hw, ir_remote* remote,
		ir_code* prep, ir_code* codep, ir_code* postp,
		int* repeat_flagp,
		lirc_t* min_remaining_gapp, lirc_t* max_remaining_gapp);

	// Send

	int (*get_send_buffer_length)(sbuf const* send_buffer);
	lirc_t const* (*get_send_buffer_data)(sbuf const* send_buffer);
	void (*init_send_buffer)(sbuf* send_buffer);
	int (*init_send)(sbuf* send_buffer, ir_remote* remote, ir_ncode* code, int repeats);
};

//
// This file describes the interface that the plugins are expected to implement.
//

#ifdef __cplusplus
    #define WL_API_EXTERNC extern "C"
#else
    #define WL_API_EXTERNC
#endif

#if defined(winlirc_EXPORTS)
    #define WL_API WL_API_EXTERNC
#else
    #define WL_API WL_API_EXTERNC __declspec(dllexport)
#endif

static const uint32_t winlirc_plugin_api_version = 1;

typedef struct ir_remote ir_remote;
typedef struct ir_ncode ir_ncode;
typedef struct hardware hardware;
typedef struct rbuf rbuf;
typedef uint64_t ir_code;
typedef int lirc_t;

#pragma pack(push, 4)
struct plugin_interface
{
	uint32_t plugin_api_version;
	int	(*init)(winlirc_interface const* wl);
	void (*deinit)(void);
	int	(*hasGui)(void);
	void (*loadSetupGui)(void);
	int	(*sendIR)(ir_remote* remote, ir_ncode* code, int32_t repeats);
	int	(*decodeIR)(ir_remote* remotes, char* out, size_t out_size);
	int	(*setTransmitters)(uint32_t transmitterMask);
	hardware const* (*getHardware)(void);
	hardware const* hardware;
};

struct hardware
{
	uint32_t plugin_api_version;

	char device[128];
	char name[128];

	uint32_t features;
	uint32_t send_mode;
	uint32_t rec_mode;
	uint32_t code_length;
	uint32_t resolution;

	int (*decode_func)(rbuf*, hardware const* hw, ir_remote* remote,
		ir_code* prep, ir_code* codep, ir_code* postp,
		int* repeat_flag,
		lirc_t* min_remaining_gapp,
		lirc_t* max_remaining_gapp);

	lirc_t(*readdata)(lirc_t timeout);
	void(*wait_for_data)(lirc_t timeout);
	int(*data_ready)(void);
	ir_code(*get_ir_code)(void);
};
#pragma pack(pop)

WL_API plugin_interface const* getPluginInterface();

WL_API int	init			(winlirc_interface const* wl);
WL_API void	deinit			();
WL_API int	hasGui			();
WL_API void	loadSetupGui	();
WL_API int	sendIR			(ir_remote* remote, ir_ncode *code, int repeats);
WL_API int	decodeIR		(ir_remote* remotes, char* out, size_t out_size);
WL_API int	setTransmitters	(unsigned int transmitterMask);
WL_API hardware const* getHardware();							// optional API for IRRecord


#define LIRC_MODE2SEND(x) (x)
#define LIRC_SEND2MODE(x) (x)
#define LIRC_MODE2REC(x) ((x) << 16)
#define LIRC_REC2MODE(x) ((x) >> 16)

#define LIRC_MODE_RAW                  0x00000001
#define LIRC_MODE_PULSE                0x00000002
#define LIRC_MODE_MODE2                0x00000004
#define LIRC_MODE_CODE                 0x00000008
#define LIRC_MODE_LIRCCODE             0x00000010
#define LIRC_MODE_STRING               0x00000020

#define LIRC_CAN_SEND_RAW              LIRC_MODE2SEND(LIRC_MODE_RAW)
#define LIRC_CAN_SEND_PULSE            LIRC_MODE2SEND(LIRC_MODE_PULSE)
#define LIRC_CAN_SEND_MODE2            LIRC_MODE2SEND(LIRC_MODE_MODE2)
#define LIRC_CAN_SEND_CODE             LIRC_MODE2SEND(LIRC_MODE_CODE)
#define LIRC_CAN_SEND_LIRCCODE         LIRC_MODE2SEND(LIRC_MODE_LIRCCODE)
#define LIRC_CAN_SEND_STRING           LIRC_MODE2SEND(LIRC_MODE_STRING)

#define LIRC_CAN_SEND_MASK             0x0000003f

#define LIRC_CAN_SET_SEND_CARRIER      0x00000100
#define LIRC_CAN_SET_SEND_DUTY_CYCLE   0x00000200
#define LIRC_CAN_SET_TRANSMITTER_MASK  0x00000400

#define LIRC_CAN_REC_RAW               LIRC_MODE2REC(LIRC_MODE_RAW)
#define LIRC_CAN_REC_PULSE             LIRC_MODE2REC(LIRC_MODE_PULSE)
#define LIRC_CAN_REC_MODE2             LIRC_MODE2REC(LIRC_MODE_MODE2)
#define LIRC_CAN_REC_CODE              LIRC_MODE2REC(LIRC_MODE_CODE)
#define LIRC_CAN_REC_LIRCCODE          LIRC_MODE2REC(LIRC_MODE_LIRCCODE)
#define LIRC_CAN_REC_STRING            LIRC_MODE2REC(LIRC_MODE_STRING)

#define LIRC_CAN_REC_MASK              LIRC_MODE2REC(LIRC_CAN_SEND_MASK)

#define LIRC_CAN_SET_REC_CARRIER       (LIRC_CAN_SET_SEND_CARRIER << 16)
#define LIRC_CAN_SET_REC_DUTY_CYCLE    (LIRC_CAN_SET_SEND_DUTY_CYCLE << 16)

#define LIRC_CAN_SET_REC_DUTY_CYCLE_RANGE 0x40000000
#define LIRC_CAN_SET_REC_CARRIER_RANGE    0x80000000
#define LIRC_CAN_GET_REC_RESOLUTION       0x20000000

#define LIRC_CAN_SEND(x) ((x)&LIRC_CAN_SEND_MASK)
#define LIRC_CAN_REC(x) ((x)&LIRC_CAN_REC_MASK)

#define LIRC_CAN_NOTIFY_DECODE            0x01000000
