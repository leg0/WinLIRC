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

#include "stdafx.h"


IrRemote::IrRemote(const wchar_t* libPath) :
	IR_Open(NULL),
	IR_Close(NULL),
	IR_GetSystemKeyCode(NULL),
	m_repeats(0),
	m_lastKey(-1)
{
	trace(L"constructor called.");

	m_lib = LoadLibrary(libPath);

	if(!m_lib)
		throw L"Loading the library failed. Is it a valid 32-bit Windows dll?";
	else if(!initFunctions(m_lib))
	{
		throw L"Sucessfully loaded the library, but it doesn't contain the right exports. Make sure you have the right library!";
		FreeLibrary(m_lib);
	}
	else
		trace(L"Loading Library was successful.");

	int32_t retval = 0;
	if(!(retval = IR_Open(0, 0, 0, 0)))
	{
		trace(L"IR_Open failed, retval: %i", retval);
		FreeLibrary(m_lib);
		throw L"I failed to open the input-device, make sure your tv-card/ir-receiver is plugged in and the drivers are installed!";
	}

	trace(L"constructor successful.");
}

IrRemote::~IrRemote()
{
	trace(L"destructor called.");
	IR_Close(0, 0);

	FreeLibrary(m_lib);
}

bool IrRemote::initFunctions(const HMODULE hMod)
{
	IR_Open             = reinterpret_cast<t_IR_Open>            (GetProcAddress(hMod, "IR_Open"));
	IR_Close            = reinterpret_cast<t_IR_Close>           (GetProcAddress(hMod, "IR_Close"));
	IR_GetSystemKeyCode = reinterpret_cast<t_IR_GetSystemKeyCode>(GetProcAddress(hMod, "IR_GetSystemKeyCode"));

	if(!IR_Open || !IR_Close || !IR_GetSystemKeyCode)
	{
		IR_Open             = NULL;
		IR_Close            = NULL;
		IR_GetSystemKeyCode = NULL;
		return false;
	}
	else
		return true;
}

bool IrRemote::getKey(char *out, size_t out_size)
{
	int32_t keyHit, repeatCode, systemCode, keyCode;
	keyHit = IR_GetSystemKeyCode(&repeatCode, &systemCode, &keyCode);
	if(keyHit)
	{
		if((repeatCode != 3) && (m_lastKey == keyCode)) //= key repeated & same key
			m_repeats++;
		else
			m_repeats = 0;

		m_lastKey = keyCode;

		trace(L"Key: kh: %i, rc: %i, sc: %i, kc: %i, repeats: %i", keyHit, repeatCode, systemCode, keyCode, m_repeats);
		
		sprintf_s(out, out_size, "%016llx %02x btn%d hauppauge%d\n", __int64(keyCode), m_repeats, keyCode, systemCode);
		return true;
	}
	return false;
}

/*int write_message(char *buffer, size_t size, const char *remote_name,
		  const char *button_name, const char *button_suffix,
		  ir_code code, int reps)
{
	int len;
	
	len=_snprintf(buffer, size, "%016llx %02x %s%s %s\n",
		     code,
		     reps,
		     button_name, button_suffix,
		     remote_name);
	return len;
}*/