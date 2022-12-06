/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
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
 * Copyright (C) 2010 Ian Curtis
 */

#include "stdafx.h"
#include "Globals.h"

Settings settings;
CWinApp app;
CIRDriver *irDriver = nullptr;

HANDLE	threadExitEvent	= nullptr;

void KillThread(CWinThread **ThreadHandle, CEvent *ThreadEvent)
{
	while(*ThreadHandle!=nullptr)
	{
		DWORD result=0;
		if(GetExitCodeThread((*ThreadHandle)->m_hThread,&result)==0) 
		{
			//DEBUG("GetExitCodeThread failed, error=%d\n",GetLastError());
			//DEBUG("(the thread may have already been terminated)\n");
			return;
		}
		if(result==STILL_ACTIVE)
		{
			ThreadEvent->SetEvent();
			if(WAIT_TIMEOUT==WaitForSingleObject((*ThreadHandle)->m_hThread,250/*INFINITE*/)) break; //maybe we just need to give it some time to quit
			ThreadEvent->ResetEvent();
			*ThreadHandle=nullptr;
		}
	}
}
