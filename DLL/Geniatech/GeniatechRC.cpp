#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#include "../Common/LIRCDefines.h"
#include "../Common/Hardware.h"
#include "../Common/WLPluginAPI.h"
#include "../Common/IRRemote.h"
#include "../Common/Linux.h"
#include "../Common/Win32Helpers.h"

#include "Globals.h"
#include "resource.h"
#include "Receive.h"
#include "Settings.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
void initHardwareStruct();
extern hardware hw;

WL_API int init(HANDLE exitEvent) {

	initHardwareStruct();
	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);

	receive = new Receive();
	return receive->init();
}

WL_API void deinit() {

	if(receive) {
		receive->deinit();
		delete receive;
		receive = NULL;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = NULL;
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
			ShowWindow(hwnd, SW_SHOW);
			return TRUE;
		}

		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK: {
					DestroyWindow (hwnd);
					return TRUE;
				}

				case IDCANCEL: {
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

	hDialog = CreateDialog((HINSTANCE)(&__ImageBase),MAKEINTRESOURCE(IDD_DIALOG_CFG),NULL,dialogProc);

    while ((status = GetMessage (&msg, 0, 0, 0)) != 0) {

        if (status == -1) return;

        if (!IsDialogMessage (hDialog, &msg)) {
            TranslateMessage ( &msg );
            DispatchMessage ( &msg );
        }
    }
}

WL_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return 0;
}

WL_API int decodeIR(struct ir_remote *remotes, char *out) {

	if(receive)
	{
		if(!receive->waitTillDataIsReady(0)) {
			return 0;
		}

		last = end;
		gettimeofday(&start,NULL);
		receive->getData(&irCode);
		gettimeofday(&end,NULL);

		if(decodeCommand(&hw,remotes,out)) {
			ResetEvent(dataReadyEvent);
			return 1;
		}

		ResetEvent(dataReadyEvent);
	}

	return 0;
}

WL_API struct hardware* getHardware() {

	initHardwareStruct();
	return &hw;
}
