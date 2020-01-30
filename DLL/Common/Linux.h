#pragma once

#include "winlirc_api.h"

// emulate a few linux functions from the lirc code base
WINLIRC_API int gettimeofday(struct mytimeval *a, void *);
