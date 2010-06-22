#include <Windows.h>
#include "Technotrend.h"
#include "Globals.h"
#include "hardware.h"
#include "Decode.h"
#include "resource.h"
#include "Receive.h"
#include <stdio.h>
#include <tchar.h>
#include "Settings.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

TE_API int init(HANDLE exitEvent) {

	init_rec_buffer();
	initHardwareStruct();

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);

	receive = new Receive();
	return receive->init(settings.getDeviceNumber(),settings.getBusyLED(),settings.getPowerLED());

}

TE_API void deinit() {

	if(dataReadyEvent) {
		CloseHandle(dataReadyEvent);
		dataReadyEvent = NULL;
	}

	if(receive) {
		receive->deinit();
		delete receive;
		receive = NULL;
	}

	threadExitEvent = NULL;

}

TE_API int hasGui() {

	return TRUE;
}

BOOL CALLBACK dialogProc (HWND hwnd, 
                          UINT message, 
                          WPARAM wParam, 
                          LPARAM lParam) {

    switch (message) {

		case WM_INITDIALOG: {

			//======================
			BYTE	numberOfDevices;
			TCHAR	temp[4];
			//======================

			numberOfDevices = irGetNumDevices();

			_stprintf(temp,_T("%i"),0);
			SendDlgItemMessage(hwnd,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)temp);

			for(int i=1; i<numberOfDevices; i++) {
				_stprintf(temp,_T("%i"),i);
				SendDlgItemMessage(hwnd,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)temp);
			}

			SendDlgItemMessage(hwnd,IDC_COMBO1,CB_SETCURSEL,settings.getDeviceNumber(),0);

			if(settings.getBusyLED())	SendDlgItemMessage(hwnd,IDC_CHECK1,BM_SETCHECK,BST_CHECKED,0);
			if(settings.getPowerLED())	SendDlgItemMessage(hwnd,IDC_CHECK2,BM_SETCHECK,BST_CHECKED,0);

			ShowWindow(hwnd, SW_SHOW);

			return TRUE;
		}

		case WM_COMMAND: {

			switch(LOWORD(wParam)) {

				case IDOK: {

					if(SendDlgItemMessage(hwnd,IDC_CHECK1,BM_GETSTATE,0,0)==BST_CHECKED) {
						settings.setBusyLED(TRUE);
					}
					else {
						settings.setBusyLED(FALSE);
					}

					if(SendDlgItemMessage(hwnd,IDC_CHECK2,BM_GETSTATE,0,0)==BST_CHECKED) {
						settings.setPowerLED(TRUE);
					}
					else {
						settings.setPowerLED(FALSE);
					}

					settings.setDeviceNumber(SendDlgItemMessage(hwnd,IDC_COMBO1,CB_GETCURSEL,0,0));

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

TE_API void	loadSetupGui() {

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

TE_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return 0;
}

TE_API int decodeIR(struct ir_remote *remotes, char *out) {

	//wait till data is ready

	if(receive) {
		receive->waitTillDataIsReady(0);
	}

	if(decodeCommand(remotes,out)) {
		return 1;
	}

	return 0;
}

TE_API struct hardware* getHardware() {

	initHardwareStruct();
	return &hw;

}