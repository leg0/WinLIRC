#pragma once

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
