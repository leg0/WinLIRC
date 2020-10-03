#pragma once

#include <chrono>

extern HANDLE threadExitEvent;
extern HANDLE dataReadyEvent;

extern CRITICAL_SECTION criticalSection;

extern std::chrono::steady_clock::time_point start, end, last;

void waitTillDataIsReady(std::chrono::microseconds maxUSecs);
