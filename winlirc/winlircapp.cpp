/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 */

#include "stdafx.h"
#include "winlircapp.h"
#include "drvdlg.h"
#include "server.h"
#include "wl_debug.h"

#include <filesystem>
#include <string_view>

namespace fs = std::filesystem;

WinLircApp app;

static fs::path getModuleFileName()
{
	wchar_t fullPath[MAX_PATH + 1];
	DWORD const pathLength = GetModuleFileNameW(nullptr, fullPath, MAX_PATH);
	return fs::path{ std::wstring_view{fullPath, pathLength} };
}

fs::path getPluginsDirectory()
{
	return absolute(getModuleFileName().replace_filename(L"plugins"));
}

BOOL WinLircApp::InitInstance() {

#ifdef _DEBUG
	AllocConsole();
#endif

	WL_DEBUG("Winlirc starting");
	AfxInitRichEdit();


	// set current directory for plugins from exe path
	fs::current_path(getPluginsDirectory());

	config.readINIFile();

	//
	// command line stuff
	//

	std::wstring_view cmdLine{ m_lpCmdLine };
	auto contains = [](std::wstring_view s, std::wstring_view b) {
		return s.find(b) != std::wstring_view::npos;
	};
	config.exitOnError = contains(cmdLine, L"/e") || contains(cmdLine, L"/E");
	config.showTrayIcon = !(contains(cmdLine, L"/t") || contains(cmdLine, L"/T"));

	wchar_t mutexName[64];
	wsprintf(mutexName, L"WinLIRC Multiple Instance Lockout_%i", config.serverPort);

	if (!CreateMutex(nullptr, FALSE, mutexName) || GetLastError()==ERROR_ALREADY_EXISTS)
	{
		HWND const winlirc = FindWindow(nullptr,_T("WinLIRC"));
		if (!winlirc)
		{
			MessageBoxW(nullptr, L"WinLIRC is already running", L"WinLIRC", MB_OK);
		}
		else
		{
			// bring it to the top
			HWND const last = GetLastActivePopup(winlirc);
			if (!IsWindowVisible(winlirc))
				ShowWindow(winlirc, SW_SHOW);

			SetForegroundWindow(winlirc);
			SetForegroundWindow(last);
		}
		return FALSE;
	}

	//
	//Process initialization and sanity checks
	//
	if(SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS)==0 || SetThreadPriority(THREAD_PRIORITY_IDLE)==0) {
		MessageBoxW(nullptr, L"Could not set thread priority.", L"WinLIRC", MB_OK|MB_ICONERROR);
		return FALSE;
	}
	
	if(!server.startServer()) {
		MessageBoxW(nullptr,L"Server could not be started. Try checking the port.", L"WinLIRC", MB_OK|MB_ICONERROR);
	}

	WL_DEBUG("Creating main dialog...\n");

	dlg.reset(new Cdrvdlg());

	if(!dlg->Create(IDD_DIALOG,nullptr)) {
		dlg.reset();
		MessageBoxW(nullptr, L"Program exiting.", L"WinLIRC", MB_OK|MB_ICONERROR);
		return FALSE;
	}

	dlg->ShowWindow(SW_HIDE);
	dlg->UpdateWindow();
	m_pMainWnd = dlg.get();
	
	return TRUE;
}

int WinLircApp::ExitInstance()
{
	dlg.reset();
	return CWinApp::ExitInstance();
}
