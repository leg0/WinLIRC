/*
Copyright (c) 2013, Philipp Borsutzki
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

-   Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
-   Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "stdafx.h"

class IrRemote
{
private:
	typedef int32_t (__stdcall *t_IR_Open)             (HWND, int32_t, uint8_t, int32_t);
	typedef int32_t (__stdcall *t_IR_Close)            (HWND, int32_t);
	typedef int32_t (__stdcall *t_IR_GetSystemKeyCode) (int32_t*, int32_t*, int32_t*);

	t_IR_Open             IR_Open;
	t_IR_Close            IR_Close;
	t_IR_GetSystemKeyCode IR_GetSystemKeyCode;

	HMODULE m_lib;
	int32_t m_lastKey;
	int	    m_repeats;

	bool initFunctions(const HMODULE hMod);
public:
	IrRemote(const wchar_t* libPath);
	~IrRemote();

	bool getKey(char *out, size_t out_size);
};