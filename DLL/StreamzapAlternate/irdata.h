#ifndef IRDATA_H
#define IRDATA_H

#include "windows.h"

extern "C" __declspec(dllimport) BOOL sz_Open(void);
extern "C" __declspec(dllimport) BOOL sz_Close(void);
extern "C" __declspec(dllimport) BOOL sz_ReadFile(BYTE *pData, LPDWORD lpNumberOfBytesRead);
extern "C" __declspec(dllimport) void sz_Flush(void);

#endif
