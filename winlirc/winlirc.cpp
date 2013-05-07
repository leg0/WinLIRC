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
		
		SetCurrentDirectory(fullPath.Left(indexOfLastSep) + _T("\\plugins\\"));
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

	dlg		= new Cdrvdlg();
	server	= new Cserver();

	if(!CreateMutex(0,FALSE,"WinLIRC Multiple Instance Lockout") || GetLastError()==ERROR_ALREADY_EXISTS) {

		//=======
		HWND tmp;
		//=======

		tmp=FindWindow(NULL,"WinLIRC");

		if(!tmp)
		{
			MessageBox(NULL,"WinLIRC is already running","WinLIRC",MB_OK);
		}
		else
		{
			// bring it to the top

			//===========
			CWnd winlirc;
			CWnd *last;
			//===========

			winlirc.Attach(tmp);

			last = winlirc.GetLastActivePopup();

			if(!winlirc.IsWindowVisible()) winlirc.ShowWindow(SW_SHOW);

			winlirc.SetForegroundWindow();
			last->SetForegroundWindow();

			winlirc.Detach();
		}
		return FALSE;
	}

	//
	//Process initialization and sanity checks
	//
	if(SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS)==0 || SetThreadPriority(THREAD_PRIORITY_IDLE)==0)
	{
		MessageBox(NULL,"Could not set thread priority.","WinLIRC",MB_OK|MB_ICONERROR);
		return FALSE;
	}
	
	if(server->init()==false) {

		MessageBox(NULL,"Could not start server.","WinLIRC",MB_OK|MB_ICONERROR);

		delete server; 
		server = NULL;

		return FALSE;
	}
	
	DEBUG("Creating main dialog...\n");

	if(!dlg->Create(IDD_DIALOG,NULL)) {

		MessageBox(NULL,"Program exiting.","WinLIRC",MB_OK|MB_ICONERROR);

		delete dlg;
		delete server; 

		dlg		= NULL; 
		server	= NULL; 
		
		return FALSE;
	}

	dlg->ShowWindow(SW_HIDE);	
	dlg->UpdateWindow();
	m_pMainWnd=dlg;
	
	return TRUE;
	
}

int Cwinlirc::ExitInstance()
{
	delete server;
	delete dlg;

	if(debugfile!=NULL) {
		fclose(debugfile);
	}

	return CWinApp::ExitInstance();
}
