#pragma once

#include <stdint.h>

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
typedef uint64_t ir_code;
typedef int lirc_t;
typedef uintptr_t WLEventHandle;

#pragma pack(push, 4)
struct plugin_interface
{
	uint32_t plugin_api_version;
	int	(*init)(WLEventHandle exitEvent);
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

	int (*decode_func)(hardware const* hw, ir_remote* remote,
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

WL_API int	init			(WLEventHandle exitEvent);
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
