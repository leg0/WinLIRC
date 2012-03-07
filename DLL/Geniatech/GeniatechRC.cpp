#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#include "GeniatechRC.h"
#include "Globals.h"
#include "resource.h"
#include "Receive.h"
#include "Settings.h"
#include "hardware.h"
#include "decode.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

IG_API int init(HANDLE exitEvent) {

	initHardwareStruct();

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);

	receive = new Receive();
	return receive->init();
}

IG_API void deinit() {

	if(receive) {
		receive->deinit();
		delete receive;
		receive = NULL;
	}

	if(dataReadyEvent) {
		CloseHandle(dataReadyEvent);
		dataReadyEvent = NULL;
	}

	threadExitEvent = NULL;
}

IG_API int hasGui() {

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

IG_API void	loadSetupGui() {

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

IG_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return 0;
}

IG_API int decodeIR(struct ir_remote *remotes, char *out) {

	//wait till data is ready

	if(receive)
	{
		receive->waitTillDataIsReady(0);

		last = end;
		gettimeofday(&start,NULL);
		receive->getData(&irCode);
		gettimeofday(&end,NULL);

		if(decodeCommand(remotes,out)) {
			return 1;
		}
	}

	return 0;
}

IG_API struct hardware* getHardware() {

	initHardwareStruct();
	return &hw;
}
