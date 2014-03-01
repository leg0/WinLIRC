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
#include "winlirc.h"
#include "drvdlg.h"
#include "server.h"
#include "guicon.h"

Cwinlirc app;

BEGIN_MESSAGE_MAP(Cwinlirc,CWinApp)
END_MESSAGE_MAP()

BOOL Cwinlirc::InitInstance() {   

	AfxInitRichEdit();
#ifdef _DEBUG
	RedirectIOToConsole();
#endif

	dlg		= NULL;
	server	= NULL;

	// set current directory for plugins from exe path

	{
		//=====================
		CString	fullPath;
		int		indexOfLastSep;
		//=====================

		GetModuleFileName(NULL, fullPath.GetBufferSetLength(MAX_PATH+1), MAX_PATH);
		indexOfLastSep = fullPath.ReverseFind(_T('\\'));
		
		if(!SetCurrentDirectory(fullPath.Left(indexOfLastSep) + _T("\\plugins\\"))) {
			SetCurrentDirectory(fullPath.Left(indexOfLastSep) + _T("\\"));
		}
	}

	config.readINIFile();

	//
	// command line stuff
	//

	if(_tcsstr(m_lpCmdLine,_T("/e")) || _tcsstr(m_lpCmdLine,_T("/E"))) {
		config.exitOnError = TRUE;
	}

	if(_tcsstr(m_lpCmdLine,_T("/t")) || _tcsstr(m_lpCmdLine,_T("/T"))) {
		config.showTrayIcon = FALSE;
	}

	dlg.reset(new Cdrvdlg());

	if(!CreateMutex(0,FALSE,_T("WinLIRC Multiple Instance Lockout")) || GetLastError()==ERROR_ALREADY_EXISTS) {

		auto winlircWindow = FindWindow(NULL, _T("WinLIRC"));

		if (!winlircWindow)
		{
			MessageBox(NULL,_T("WinLIRC is already running"),_T("WinLIRC"),MB_OK);
		}
		else
		{
			// bring it to the top
			auto last = ::GetLastActivePopup(winlircWindow);

			if (!::IsWindowVisible(winlircWindow))
				::ShowWindow(winlircWindow, SW_SHOW);

			::SetForegroundWindow(winlircWindow);
			::SetForegroundWindow(last);
		}
		return FALSE;
	}

	//
	//Process initialization and sanity checks
	//
	if(SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS)==0 || SetThreadPriority(THREAD_PRIORITY_IDLE)==0)
	{
		MessageBox(NULL,_T("Could not set thread priority."),_T("WinLIRC"),MB_OK|MB_ICONERROR);
		return FALSE;
	}
	
	WL_DEBUG("Creating main dialog...\n");

	if(!dlg->Create(IDD_DIALOG,NULL)) {

		MessageBox(NULL,_T("Program exiting."),_T("WinLIRC"),MB_OK|MB_ICONERROR);

		dlg.reset();
		server.reset(); 
		
		return FALSE;
	}

	dlg->ShowWindow(SW_HIDE);
	dlg->UpdateWindow();
	m_pMainWnd = dlg.get();

	server = std::make_shared<Cserver>(*dlg);
	dlg->setNetworkEventHandler(server);
	if (!server->startServer()) {
		MessageBox(NULL, _T("Server could not be started. Try checking the port."), _T("WinLIRC"), MB_OK | MB_ICONERROR);
	}

	return TRUE;
	
}

int Cwinlirc::ExitInstance()
{
	server.reset();
	dlg.reset();

	return CWinApp::ExitInstance();
}
