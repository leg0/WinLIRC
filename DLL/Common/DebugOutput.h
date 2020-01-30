#pragma once

#if defined(_DEBUG) || defined (DEBUG)

	#include <stdio.h>
	#include <tchar.h>

	#define DPRINTF printf
	#define DWPRINTF wprintf

	#ifdef _UNICODE
		#define TPRINTF wprintf
	#else
		#define TPRINTF wprintf
	#endif
#else
	#define DPRINTF(a, ...)
	#define DWPRINTF(a, ...)
	#define TPRINTF(a, ...)
#endif
