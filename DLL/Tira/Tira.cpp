#include <Windows.h>
#include <stdio.h>

#include "TiraDLL.h"
#include "resource.h"
#include "Settings.h"
#include "Globals.h"

#include <winlirc/WLPluginAPI.h>
#include <winlirc/winlirc_api.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
void initHardwareStruct();
extern hardware hw;
extern rbuf rec_buffer;

//===================
TiraDLL		tiraDLL;
Settings	settings;
//===================

int WINAPI tiraCallbackFunction(const char * eventstring);

WL_API int init(winlirc_api const* winlirc) {

	if(tiraDLL.tira_init()!=TIRA_TRUE) 
		return 0;

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);

	InitializeCriticalSection(&criticalSection);

	end = std::chrono::steady_clock::now();

	if(tiraDLL.tira_start(settings.getComPort())!=TIRA_TRUE) 
		return 0;

	if(tiraDLL.tira_set_handler(tiraCallbackFunction)!=TIRA_TRUE) 
		return 0;

	return 1;
}

WL_API void deinit() {

	tiraDLL.tira_stop();
	tiraDLL.tira_cleanup();

	if(dataReadyEvent) {
		CloseHandle(dataReadyEvent);
		dataReadyEvent = nullptr;
	}

	threadExitEvent = nullptr;

	DeleteCriticalSection(&criticalSection);
}

WL_API int hasGui() {

	return TRUE;
}

INT_PTR CALLBACK dialogProc (HWND hwnd, 
                             UINT message, 
                             WPARAM wParam, 
                             LPARAM lParam) {

    switch (message) {

		case WM_INITDIALOG: {

			//=========================
			HWND dialogItem;
			const char* version = nullptr;
			//=========================

			tiraDLL.tira_init();

			version		= tiraDLL.tira_get_version(0);
			dialogItem	= GetDlgItem(hwnd,IDC_DLLVERSION);

			if(dialogItem)	SetWindowTextA(dialogItem,version);
			else			SetWindowTextA(dialogItem,"Information missing");

			version		= tiraDLL.tira_get_version(1);
			dialogItem	= GetDlgItem(hwnd,IDC_FIRMWAREVERSION);

			if(dialogItem)	SetWindowTextA(dialogItem,version);
			else			SetWindowTextA(dialogItem,"Information missing");

			SetDlgItemInt(hwnd,IDC_EDIT1,settings.getComPort()+1,FALSE);

			ShowWindow(hwnd, SW_SHOW);

			return TRUE;
		}


		case WM_COMMAND: {

			switch(LOWORD(wParam)) {

				case IDOK: {

					//=======
					int temp;
					//=======

					temp = GetDlgItemInt(hwnd,IDC_EDIT1,nullptr,FALSE);
					temp = temp - 1;

					if(temp<0) temp = 0;

					settings.setComPort(temp);
					settings.saveSettings();

					DestroyWindow (hwnd);
					return TRUE;
				}

				case IDCANCEL: {
					//
					//ignore changes
					//
					DestroyWindow (hwnd);
					return TRUE;
				}

			}

			return FALSE;

		}
		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;
		case WM_CLOSE:
			DestroyWindow (hwnd);
			return TRUE;
	}

    return FALSE;

}

WL_API void	loadSetupGui() {

	//==============
	HWND	hDialog;
	MSG		msg;
    INT		status;
	//==============

	hDialog = CreateDialog((HINSTANCE)(&__ImageBase),MAKEINTRESOURCE(IDD_DIALOG1),nullptr,dialogProc);

    while ((status = GetMessage (& msg, 0, 0, 0)) != 0) {

        if (status == -1) return;

        if (!IsDialogMessage (hDialog, & msg)) {

            TranslateMessage ( & msg );
            DispatchMessage ( & msg );
        }
    }

}

WL_API int sendIR(struct ir_remote *remotes, struct ir_ncode *code, int repeats) {

	//
	// return false - since we don't support this function yet .. Tira should be able to send though
	//

	return 0;
}

extern bool waitForData(std::chrono::microseconds timeout);

WL_API int decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	last = end;

	start = std::chrono::steady_clock::now();

	using namespace std::chrono_literals;
	if(!waitForData(0us)) {
		return 0;
	}

	end = std::chrono::steady_clock::now();

	if(winlirc_decodeCommand(&rec_buffer, &hw,remotes,out,out_size)) {
		ResetEvent(dataReadyEvent);
		return 1;
	}

	ResetEvent(dataReadyEvent);

	return 0;
}

WL_API hardware const* getHardware() {

	initHardwareStruct();
	return &hw;
}