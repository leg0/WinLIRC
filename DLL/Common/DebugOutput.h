#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

#if defined(_DEBUG) || defined (DEBUG)
	#include <stdio.h>
	#define DPRINTF printf
	#define DWPRINTF wprintf
#else
	#define DPRINTF(a, ...)
	#define DWPRINTF(a, ...)
#endif

#endif
