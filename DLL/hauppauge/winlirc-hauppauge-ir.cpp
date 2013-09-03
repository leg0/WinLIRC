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
#include "../Common/WLPluginAPI.h"
#include "irremote.h"

const wchar_t *g_cPluginName = L"hauppauge-irremote";
const wchar_t *g_cLibPathStr = L"libraryPath";
wchar_t g_libPath[MAX_PATH] = {};

IrRemote *g_ir = NULL;
HANDLE g_exitEvent = 0;

bool setLibraryPath(const wchar_t *path)
{
	wchar_t currentDirectory[MAX_PATH];
	FILE  *file;

	GetCurrentDirectory(MAX_PATH,currentDirectory);
	wcscat_s(currentDirectory, MAX_PATH - 1, L"\\WinLIRC.ini");

	// if our ini files doesn't exist try and create it
	_wfopen_s(&file, currentDirectory, L"r");

	if(!file)
		_wfopen_s(&file, currentDirectory, L"w");

	if(file)
		fclose(file);

	return WritePrivateProfileString(g_cPluginName, g_cLibPathStr, path, currentDirectory) != FALSE;
}

bool getLibraryPath(wchar_t *path, size_t maxPathLen)
{
	wchar_t currentDirectory[MAX_PATH];

	GetCurrentDirectory(MAX_PATH,currentDirectory);
	wcscat_s(currentDirectory, MAX_PATH - 1, L"\\WinLIRC.ini");

	return GetPrivateProfileString(g_cPluginName, g_cLibPathStr, NULL, path, maxPathLen, currentDirectory) != FALSE;
}

///////////////////////
// WinLIRC-Interface //
///////////////////////

WL_API int init(HANDLE exitEvent)
{
	trace(L"init");
	g_exitEvent = exitEvent;

	try
	{
		if(!getLibraryPath(g_libPath, _countof(g_libPath)))
			throw L"Library-Path not set within WinLIRC.ini, please set it in the config dialog for this plugin!";

		g_ir = new IrRemote(g_libPath);
	}
	catch(const wchar_t *error)
	{
		wchar_t msg[1024];
		swprintf_s(msg, 1023, L"Error:\n\n%s", error);
		MessageBox(0, msg, L"Hauppauge IR Remote - Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	catch(...)
	{
		trace(L"Unknown error.");
		MessageBox(0, L"Unknown error caught!", L"Hauppauge IR Remote - Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	return TRUE;
}

WL_API void	deinit()
{
	trace(L"deinit");

	if(g_ir)
	{
		delete g_ir;
		g_ir = NULL;
	}
}

WL_API int hasGui()
{
	trace(L"hasGui");
	return TRUE;
}

WL_API void	loadSetupGui()
{
	trace(L"loadSetupGui");

	const wchar_t *filePickerFilter =
		L"irremote.dll\0irremote.dll\0"
		L"All Library Files (*.dll)\0*.dll\0"
		L"All Files (*)\0*\0";
	wchar_t fileName[MAX_PATH] = {};
	const HWND hwnd = 0;

	MessageBox(hwnd, L"Please choose the irremote.dll library from your Hauppauge WinTV directory in the following dialog! ", L"Hauppauge IR Remote", MB_ICONINFORMATION | MB_OK);

	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = filePickerFilter;
	ofn.lpstrTitle = L"Please choose the irremote.dll from your Hauppauge WinTV directory!";
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = fileName;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if(!GetOpenFileName(&ofn))
		return;

	try
	{
		IrRemote ir(fileName);

		//if we get here, everything worked
		setLibraryPath(fileName);

		MessageBox(hwnd, L"Library seems to be ok, opening the receiver worked too.", L"Hauppauge IR Remote", MB_ICONINFORMATION | MB_OK);
	}
	catch(const wchar_t *error)
	{
		wchar_t msg[1024];
		swprintf_s(msg, 1023, L"Error:\n\n%s", error);
		MessageBox(hwnd, msg, L"Hauppauge IR Remote - Error", MB_ICONERROR | MB_OK);
		return;
	}
	catch(...)
	{
		trace(L"Unknown error.");
		MessageBox(hwnd, L"Unknown error caught!", L"Hauppauge IR Remote - Error", MB_ICONERROR | MB_OK);
		return;
	}
}

WL_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats)
{
	trace(L"sendIR");
	return FALSE;
}

WL_API int decodeIR(struct ir_remote *remotes, char *out)
{
	trace(L"decodeIR");

	if(g_ir)
	{
		while(!g_ir->getKey(out))
			if(WaitForSingleObject(g_exitEvent, 100) == WAIT_OBJECT_0) //= sleep(100)
			{
				trace(L"exitEvent signaled");
				return FALSE;
			}
		return TRUE;
	}
	else
		trace(L"called without correct initialization!");
	return FALSE;
}