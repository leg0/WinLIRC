#include "stdafx.h"
#include "Globals.h"
#include "App.h"

Settings settings;
App app;
CIRDriver *irDriver = NULL;

HANDLE	threadExitEvent	= NULL;

void KillThread(CWinThread **ThreadHandle, CEvent *ThreadEvent)
{
	while(*ThreadHandle!=NULL)
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
			*ThreadHandle=NULL;
		}
	}
}
