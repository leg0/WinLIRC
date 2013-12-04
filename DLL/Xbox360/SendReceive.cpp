#include "SendReceive.h"
#include <stdio.h>
#include "../Common/LIRCDefines.h"

DWORD WINAPI XBThread(void *recieveClass) {

	((SendReceive*)recieveClass)->threadProc();
	return 0;
}

SendReceive::SendReceive() {

	threadHandle	= NULL;
	dataReadyEvent	= NULL;
	threadExitEvent	= NULL;
}

BOOL SendReceive::init(HANDLE exit) {

	threadExitEvent	= exit;
	dataReadyEvent	= CreateEvent(NULL,TRUE,FALSE,NULL);
	threadHandle	= CreateThread(NULL,0,XBThread,(void *)this,0,NULL);
	
	if(threadHandle && dataReadyEvent) {
		return TRUE;
	}

	return FALSE;
}

void SendReceive::deinit() {

	killThread();

	if(dataReadyEvent) {
		CloseHandle(dataReadyEvent);
		dataReadyEvent = NULL;
	}
}

void SendReceive::threadProc() {

	value		= 0;
	repeats		= 0;
	done		= FALSE;

	while(!done) {

		//============================
		XINPUT_KEYSTROKE	keyStroke;
		DWORD				result;
		//============================

		result = XInputGetKeystroke(XUSER_INDEX_ANY,XINPUT_FLAG_GAMEPAD,&keyStroke);

		if(result==ERROR_SUCCESS) {

			if(keyStroke.Flags&XINPUT_KEYSTROKE_KEYDOWN) {

				value = keyStroke.VirtualKey;

				if(keyStroke.Flags&XINPUT_KEYSTROKE_REPEAT) {
					repeats++;
				}
				else {
					repeats = 0;
				}
				
				SetEvent(dataReadyEvent);
			}
		}
		else if(result==ERROR_EMPTY) {
			Sleep(50);
		}
		else if(result==ERROR_DEVICE_NOT_CONNECTED) {
			Sleep(500);
		}		
	}
}

bool SendReceive::waitTillDataIsReady(int maxUSecs) {

	//================
	HANDLE	events[2];
	int		count;
	//================

	events[0] = dataReadyEvent;
	events[1] = threadExitEvent;

	count = 2;

	if(threadExitEvent==NULL) {
		count = 1;
	}

	if(!dataReady()) {

		//=========
		int result;
		//=========

		ResetEvent(dataReadyEvent);

		if(maxUSecs) {
			result = WaitForMultipleObjects(count,events,FALSE,(maxUSecs+500)/1000);
		}
		else {
			result = WaitForMultipleObjects(count,events,FALSE,INFINITE);
		}

		if(result==(WAIT_OBJECT_0+1)) {
			return false;
		}
	}

	return true;
}

void SendReceive::killThread() {

	done = TRUE;

	if(threadHandle!=NULL) {

		//===========
		DWORD result;
		//===========

		result = 0;

		if(GetExitCodeThread(threadHandle,&result)==0) 
		{
			CloseHandle(threadHandle);
			threadHandle = NULL;
			return;
		}

		if(result==STILL_ACTIVE)
		{
			WaitForSingleObject(threadHandle,INFINITE);
		}

		CloseHandle(threadHandle);
		threadHandle = NULL;
	}
}

bool SendReceive::dataReady() {

	if(value==0) return false;

	return true;
}

int SendReceive::decodeCommand(char *out) {

	//==================
	char buttonName[32];
	//==================

	switch(value) {

		case VK_PAD_A:					strcpy_s(buttonName,"A");				break;
		case VK_PAD_B:					strcpy_s(buttonName,"B");				break;
		case VK_PAD_X:					strcpy_s(buttonName,"X");				break;
		case VK_PAD_Y:					strcpy_s(buttonName,"Y");				break;

		case VK_PAD_RSHOULDER:			strcpy_s(buttonName,"RSHOULDER");		break;
		case VK_PAD_LSHOULDER:			strcpy_s(buttonName,"LSHOULDER");		break;
		case VK_PAD_LTRIGGER:			strcpy_s(buttonName,"LTRIGGER");		break;
		case VK_PAD_RTRIGGER:			strcpy_s(buttonName,"RTRIGGER");		break;

		case VK_PAD_DPAD_UP:			strcpy_s(buttonName,"DPAD_UP");			break;
		case VK_PAD_DPAD_DOWN:			strcpy_s(buttonName,"DPAD_DOWN");		break;
		case VK_PAD_DPAD_LEFT:			strcpy_s(buttonName,"DPAD_LEFT");		break;
		case VK_PAD_DPAD_RIGHT:			strcpy_s(buttonName,"DPAD_RIGHT");		break;

		case VK_PAD_START:				strcpy_s(buttonName,"START");			break;
		case VK_PAD_BACK:				strcpy_s(buttonName,"BACK");			break;

		case VK_PAD_LTHUMB_PRESS:		strcpy_s(buttonName,"LTHUMB_PRESS");	break;
		case VK_PAD_RTHUMB_PRESS:		strcpy_s(buttonName,"RTHUMB_PRESS");	break;

		case VK_PAD_LTHUMB_UP:			strcpy_s(buttonName,"LTHUMB_UP");		break;
		case VK_PAD_LTHUMB_DOWN:		strcpy_s(buttonName,"LTHUMB_DOWN");		break;
		case VK_PAD_LTHUMB_RIGHT:		strcpy_s(buttonName,"LTHUMB_RIGHT");	break;
		case VK_PAD_LTHUMB_LEFT:		strcpy_s(buttonName,"LTHUMB_LEFT");		break;
		case VK_PAD_LTHUMB_UPLEFT:		strcpy_s(buttonName,"LTHUMB_UPLEFT");	break;
		case VK_PAD_LTHUMB_UPRIGHT:		strcpy_s(buttonName,"LTHUMB_UPRIGHT");	break;
		case VK_PAD_LTHUMB_DOWNRIGHT:	strcpy_s(buttonName,"LTHUMB_DOWNRIGHT");break;
		case VK_PAD_LTHUMB_DOWNLEFT:	strcpy_s(buttonName,"LTHUMB_DOWNLEFT");	break;

		case VK_PAD_RTHUMB_UP:			strcpy_s(buttonName,"RTHUMB_UP");		break;
		case VK_PAD_RTHUMB_DOWN:		strcpy_s(buttonName,"RTHUMB_DOWN");		break;
		case VK_PAD_RTHUMB_RIGHT:		strcpy_s(buttonName,"RTHUMB_RIGHT");	break;
		case VK_PAD_RTHUMB_LEFT:		strcpy_s(buttonName,"RTHUMB_LEFT");		break;
		case VK_PAD_RTHUMB_UPLEFT:		strcpy_s(buttonName,"RTHUMB_UPLEFT");	break;
		case VK_PAD_RTHUMB_UPRIGHT:		strcpy_s(buttonName,"RTHUMB_UPRIGHT");	break;
		case VK_PAD_RTHUMB_DOWNRIGHT:	strcpy_s(buttonName,"RTHUMB_DOWNRIGHT");break;
		case VK_PAD_RTHUMB_DOWNLEFT:	strcpy_s(buttonName,"RTHUMB_DOWNLEFT");	break;

		default:
			{
				value = 0;
				return 0;
			}
	}

	_snprintf_s(out,PACKET_SIZE+1,PACKET_SIZE+1,"%016llx %02x %s %s\n",__int64(0),repeats,buttonName,"Xbox360");

	value = 0;
	
	return 1;
}

