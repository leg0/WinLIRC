#ifndef GLOBALS_H
#define GLOBALS_H

#include <Windows.h>
#include <sys/timeb.h>
#include "Receive.h"
#include "Settings.h"

extern HANDLE threadExitEvent;
extern HANDLE dataReadyEvent;

extern Receive *receive;
extern Settings settings;

extern struct mytimeval start,end,last;

extern ir_code irCode;

int gettimeofday(struct mytimeval *a, void *);

#endif

