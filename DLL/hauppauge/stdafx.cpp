// stdafx.cpp : Quelldatei, die nur die Standard-Includes einbindet.
// winlirc-hauppauge-ir.pch ist der vorkompilierte Header.
// stdafx.obj enthält die vorkompilierten Typinformationen.

#include "stdafx.h"

// TODO: Auf zusätzliche Header verweisen, die in STDAFX.H
// und nicht in dieser Datei erforderlich sind.


HINSTANCE hInstance = NULL;

void XTrace0(LPCTSTR lpszText)
{
   ::OutputDebugString(lpszText);
}

void XTrace(LPCTSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);
    int nBuf;
    static TCHAR szBuffer[1024]; // get rid of this hard-coded buffer
    nBuf = _vsnwprintf_s(szBuffer, 1023, lpszFormat, args);
    ::OutputDebugString(szBuffer);
    va_end(args);
}