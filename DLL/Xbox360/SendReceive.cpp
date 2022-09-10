#include "SendReceive.h"
#include <stdio.h>
#include <winlirc-common/Win32Helpers.h>

DWORD WINAPI XBThread(void *recieveClass) {

	((SendReceive*)recieveClass)->threadProc();
	return 0;
}

SendReceive::SendReceive() {

	m_threadHandle		= nullptr;
	m_dataReadyEvent	= nullptr;
	m_threadExitEvent	= nullptr;
}

BOOL SendReceive::init(HANDLE exit) {

	m_threadExitEvent	= exit;
	m_dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);
	m_threadHandle		= CreateThread(nullptr,0,XBThread,(void *)this,0,nullptr);
	
	if(m_threadHandle && m_dataReadyEvent) {
		return TRUE;
	}

	return FALSE;
}

void SendReceive::deinit() {

	m_done = TRUE;

	KillThread(nullptr,m_threadHandle);

	SAFE_CLOSE_HANDLE(m_dataReadyEvent);
}

void SendReceive::threadProc() {

	m_value		= 0;
	m_repeats	= 0;
	m_done		= FALSE;

	while(!m_done) {

		//============================
		XINPUT_KEYSTROKE	keyStroke;
		DWORD				result;
		//============================

		result = XInputGetKeystroke(XUSER_INDEX_ANY,XINPUT_FLAG_GAMEPAD,&keyStroke);

		if(result==ERROR_SUCCESS) {

			if(keyStroke.Flags&XINPUT_KEYSTROKE_KEYDOWN) {

				m_value = keyStroke.VirtualKey;

				if(keyStroke.Flags&XINPUT_KEYSTROKE_REPEAT) {
					m_repeats++;
				}
				else {
					m_repeats = 0;
				}
				
				SetEvent(m_dataReadyEvent);
			}
		}
		else if(result==ERROR_EMPTY) {
			Sleep(50);
		}
		else if(result==ERROR_DEVICE_NOT_CONNECTED) {
			Sleep(1000);
		}		
	}
}

bool SendReceive::waitTillDataIsReady(std::chrono::microseconds maxUSecs) {

	HANDLE const events[] = { m_dataReadyEvent, m_threadExitEvent };
	DWORD const count = (m_threadExitEvent == nullptr) ? 1 : 2;

	if(!dataReady()) {

		ResetEvent(m_dataReadyEvent);

		using namespace std::chrono;
		DWORD const dwTimeout = maxUSecs > 0us
			? duration_cast<milliseconds>(maxUSecs + 500us).count()
			: INFINITE;
		DWORD const result = ::WaitForMultipleObjects(count, events, false, dwTimeout);

		if(result==(WAIT_OBJECT_0+1)) {
			return false;
		}
	}

	return true;
}

bool SendReceive::dataReady() {

	if(m_value==0) return false;

	return true;
}

int SendReceive::decodeCommand(char *out, size_t out_size) {

	//==================
	char buttonName[32];
	//==================

	switch(m_value) {

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
				m_value = 0;
				return 0;
			}
	}

	_snprintf_s(out,out_size,out_size,"%016llx %02x %s %s\n",__int64(0),m_repeats,buttonName,"Xbox360");

	m_value = 0;
	
	return 1;
}

