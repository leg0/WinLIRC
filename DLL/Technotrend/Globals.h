#ifndef GLOBALS_H
#define GLOBALS_H

#include <Windows.h>
#include "Receive.h"
#include "Settings.h"

extern HANDLE threadExitEvent;
extern HANDLE dataReadyEvent;

extern Receive *receive;
extern Settings settings;

#endif

