#pragma once

#include <Windows.h>
#include "Receive.h"
#include "Settings.h"
#include <chrono>

extern HANDLE threadExitEvent;
extern HANDLE dataReadyEvent;

extern Receive *receive;
extern Settings settings;

extern std::chrono::steady_clock::time_point start,end,last;

extern ir_code irCode;
