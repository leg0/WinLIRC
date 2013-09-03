#pragma once

#include "targetver.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <string>

#ifndef _STDINT
	typedef signed char int8_t;
	typedef short int16_t;
	typedef int int32_t;

	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
	typedef unsigned int uint32_t;
#endif

#include "irremote.h"
#include "../Common/LIRCDefines.h" //get this from winlirc ( http://sourceforge.net/projects/winlirc/ ) 

extern HINSTANCE hInstance;

void XTrace0(LPCTSTR lpszText);
void XTrace(LPCTSTR lpszFormat, ...);

#ifdef _DEBUG
# define __FILEW_SHORT__ (wcsrchr(__FILEW__, '\\') ? wcsrchr(__FILEW__, '\\') + 1 : __FILEW__)
# define trace(format, ...) XTrace(L"File: %s, Line: %i, Func: %s: " ## format ## L"\n", __FILEW_SHORT__, __LINE__, __FUNCTIONW__, __VA_ARGS__)
#else
# define trace(...) 
#endif
