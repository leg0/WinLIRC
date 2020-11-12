#pragma once

#ifdef _DEBUG
	#define WL_DEBUG(...) winlirc_debug(__FILE__,__LINE__,__VA_ARGS__)
	extern void winlirc_debug(const char* file, int line, char const* format, ...);
#else
	#define WL_DEBUG(a, ...)
#endif
