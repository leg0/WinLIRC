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
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include "afxmt.h"
#include <stdarg.h>
#include "irconfig.h"

/* Debugging stuff */
bool debug=false;
FILE *debugfile=NULL;
char *__file="?";
int __line=0;
void winlirc_debug(char *format, ...)
{
	va_list args;
	va_start(args,format);

	char s[1024];
	char t[512];
	_vsnprintf(t,512,format,args);
	_snprintf(s,1024,"%s(%d) : %s",__file,__line,t);

#ifdef _DEBUG
	afxDump << s;
#else
	fprintf(debugfile,s);
	fflush(debugfile);
#endif
}


/* End of Debugging stuff */

struct ir_remote *global_remotes=NULL;

CWinThread *ServerThreadHandle=NULL;
CEvent ServerThreadEvent;

CCriticalSection CS_global_remotes;

CIRConfig config;

void KillThread(CWinThread **ThreadHandle, CEvent *ThreadEvent)
{
	while(*ThreadHandle!=NULL)
	{
		DWORD result=0;
		if(GetExitCodeThread((*ThreadHandle)->m_hThread,&result)==0) 
		{
			DEBUG("GetExitCodeThread failed, error=%d\n",GetLastError());
			DEBUG("(the thread may have already been terminated)\n");
			return;
		}
		if(result==STILL_ACTIVE)
		{
			//printf("still active\n");
			ThreadEvent->SetEvent();
			if(WAIT_TIMEOUT==WaitForSingleObject((*ThreadHandle)->m_hThread,250)) break; //maybe we just need to give it some time to quit
			ThreadEvent->ResetEvent();
			*ThreadHandle=NULL;
		}
	}
}

void KillThread2(CWinThread **ThreadHandle, HANDLE ThreadEvent)
{
	while(*ThreadHandle!=NULL)
	{
		DWORD result=0;
		if(GetExitCodeThread((*ThreadHandle)->m_hThread,&result)==0) 
		{
			DEBUG("GetExitCodeThread failed, error=%d\n",GetLastError());
			DEBUG("(the thread may have already been terminated)\n");
			return;
		}
		if(result==STILL_ACTIVE)
		{
			//printf("still active\n");
			SetEvent(ThreadEvent);
			if(WaitForSingleObject((*ThreadHandle)->m_hThread, 5000) == WAIT_TIMEOUT) {
				// The plug-in does not seem to respect the exitEvent. Kill it!
				TerminateThread((*ThreadHandle)->m_hThread, 999);
			}
			ResetEvent(ThreadEvent);
			*ThreadHandle=NULL;
		}
	}
}
