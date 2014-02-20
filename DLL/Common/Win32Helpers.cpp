#include <Windows.h>
#include "Win32Helpers.h"

void KillThread(HANDLE exitEvent, HANDLE &threadHandle) {

	if(exitEvent) {
		SetEvent(exitEvent);
	}

	if(threadHandle!=NULL) {

		//===========
		DWORD result;
		//===========

		result = 0;

		if(GetExitCodeThread(threadHandle,&result)==0) {
			SAFE_CLOSE_HANDLE(threadHandle);
			return;
		}

		if(result==STILL_ACTIVE) {
			WaitForSingleObject(threadHandle,INFINITE);
		}

		SAFE_CLOSE_HANDLE(threadHandle);
	}
}