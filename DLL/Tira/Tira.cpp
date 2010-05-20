#include <Windows.h>
#include "Tira.h"
#include "TiraDLL.h"
#include "resource.h"
#include "Settings.h"
#include "Globals.h"
#include "Decode.h"
#include "LIRCDefines.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

//===================
TiraDLL		tiraDLL;
Settings	settings;
//===================

TI_API int init(HANDLE exitEvent) {

	if(tiraDLL.tira_init()!=TIRA_TRUE) return 0;

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);

	InitializeCriticalSection(&criticalSection);

	gettimeofday(&end,NULL); // initialise to something meaninful

	tiraDLL.tira_start(settings.getComPort());
	tiraDLL.tira_set_handler(tiraCallbackFunction);

	return 1;
}

TI_API void deinit() {

	tiraDLL.tira_stop();
	tiraDLL.tira_cleanup();

	if(dataReadyEvent) {
		CloseHandle(dataReadyEvent);
		dataReadyEvent = NULL;
	}

	threadExitEvent = NULL;

	DeleteCriticalSection(&criticalSection);

}

TI_API int hasGui() {

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

			version		= tiraDLL.tira_get_version(0);
			dialogItem	= GetDlgItem(hwnd,IDC_DLLVERSION);//->SetWindowTextA(version);

			if(dialogItem)	SetWindowTextA(dialogItem,version);
			else			SetWindowTextA(dialogItem,"Information missing");

			version		= tiraDLL.tira_get_version(1);
			dialogItem	= GetDlgItem(hwnd,IDC_FIRMWAREVERSION);//->SetWindowTextA(version);

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

TI_API void	loadSetupGui() {

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

TI_API int sendIR(struct ir_remote *remotes, struct ir_ncode *code, int repeats) {

	//
	// return false - since we don't support this function yet .. Tira should be able to send though
	//

	return 0;
}

TI_API int decodeIR(struct ir_remote *remotes, char *out) {

	last = end;

	gettimeofday		(&start,NULL);
	waitTillDataIsReady	(0);
	gettimeofday		(&end,NULL);

	if(decodeCommand(remotes,out)) {
		return 1;
	}

	return 0;
}

TI_API struct hardware* getHardware() {

	return 0;

}