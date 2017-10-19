#ifndef _LINUX_H_
#define _LINUX_H_

#include "winlirc_api.h"

// emulate a few linux functions from the lirc code base
WINLIRC_API int gettimeofday(struct mytimeval *a, void *);

#endif