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
#include <stdarg.h>
#include "irconfig.h"

/* Debugging stuff */

void winlirc_debug(const char *file, int line, char *format, ...)
{
	//=========
	CStringA s;
	CStringA t;
	//=========

	va_list args;
	va_start(args,format);

	s.Format("%s(%i) : ",file,line);
	t.FormatV(format,args);

	s += t;

	printf(s);
}

/* End of Debugging stuff */

ir_remote* global_remotes = nullptr;

std::mutex CS_global_remotes;

CIRConfig config;

void KillThread(CWinThread **ThreadHandle, CEvent *ThreadEvent)
{
	while(*ThreadHandle!=nullptr)
	{
		DWORD result=0;
		if(GetExitCodeThread((*ThreadHandle)->m_hThread,&result)==0) 
		{
			WL_DEBUG("GetExitCodeThread failed, error=%d\n",GetLastError());
			WL_DEBUG("(the thread may have already been terminated)\n");
			return;
		}
		if(result==STILL_ACTIVE)
		{
			//printf("still active\n");
			ThreadEvent->SetEvent();
			if(WAIT_TIMEOUT==WaitForSingleObject((*ThreadHandle)->m_hThread,250)) break; //maybe we just need to give it some time to quit
			ThreadEvent->ResetEvent();
			*ThreadHandle=nullptr;
		}
	}
}
