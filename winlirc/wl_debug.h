#pragma once

#ifdef _DEBUG
	#include <source_location>
	#define WL_DEBUG(...) winlirc_debug(std::source_location::current(),__VA_ARGS__)
	extern void winlirc_debug(std::source_location loc, char const* format, ...);
#else
	#define WL_DEBUG(...)
#endif
