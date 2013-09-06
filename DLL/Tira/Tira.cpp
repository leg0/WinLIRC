#include <Windows.h>
#include <stdio.h>

#include "TiraDLL.h"
#include "resource.h"
#include "Settings.h"
#include "Globals.h"

#include "../Common/LIRCDefines.h"
#include "../Common/Hardware.h"
#include "../Common/WLPluginAPI.h"
#include "../Common/IRRemote.h"
#include "../Common/Linux.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

//===================
TiraDLL		tiraDLL;
Settings	settings;
//===================

int WINAPI tiraCallbackFunction(const char * eventstring);

WL_API int init(HANDLE exitEvent) {

	if(tiraDLL.tira_init()!=TIRA_TRUE) 
		return 0;

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,TRUE,FALSE,NULL);

	InitializeCriticalSection(&criticalSection);

	gettimeofday(&end,NULL); // initialise to something meaninful

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
		dataReadyEvent = NULL;
	}

	threadExitEvent = NULL;

	DeleteCriticalSection(&criticalSection);
}

WL_API int hasGui() {

	return TRUE;
}

BOOL CALLBACK dialogProc (HWND hwnd, 
                          UINT message, 
                          WPARAM wParam, 
                          LPARAM lParam) {

    switch (message) {

		case WM_INITDIALOG: {

			//=========================
			HWND dialogItem;
			const char* version = NULL;
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

					temp = GetDlgItemInt(hwnd,IDC_EDIT1,NULL,FALSE);
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

	hDialog = CreateDialog((HINSTANCE)(&__ImageBase),MAKEINTRESOURCE(IDD_DIALOG1),NULL,dialogProc);

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

WL_API int decodeIR(struct ir_remote *remotes, char *out) {

	last = end;

	gettimeofday		(&start,NULL);
	hw.wait_for_data	(0);
	gettimeofday		(&end,NULL);

	if(decodeCommand(remotes,out)) {
		ResetEvent(dataReadyEvent);
		return 1;
	}

	ResetEvent(dataReadyEvent);

	return 0;
}

WL_API struct hardware* getHardware() {

	return &hw;
}