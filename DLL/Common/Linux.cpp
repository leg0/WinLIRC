#include "Linux.h"
#include "LIRCDefines.h"
#include <sys/timeb.h>

WINLIRC_API int gettimeofday(mytimeval* a, void*)
/* only accurate to milliseconds, instead of microseconds */
{
	_timeb tstruct;
	_ftime_s(&tstruct);
	
	a->tv_sec=tstruct.time;
	a->tv_usec=tstruct.millitm*1000;

	return 1;
}