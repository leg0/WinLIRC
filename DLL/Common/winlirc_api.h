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
