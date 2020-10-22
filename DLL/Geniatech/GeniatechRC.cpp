#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#include <winlirc/WLPluginAPI.h>
#include <winlirc/winlirc_api.h>
#include "../Common/Win32Helpers.h"

#include "Globals.h"
#include "resource.h"
#include "Receive.h"
#include "Settings.h"

#include <chrono>

using namespace std::chrono;

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
void initHardwareStruct();
extern hardware hw;

WL_API int init(WLEventHandle exitEvent) {

	initHardwareStruct();
	threadExitEvent = reinterpret_cast<HANDLE>(exitEvent);
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

INT_PTR CALLBACK dialogProc (HWND hwnd, 
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

WL_API int decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(receive)
	{
		if(!receive->waitTillDataIsReady(0)) {
			return 0;
		}

		last = end;
		start = steady_clock::now();
		receive->getData(&irCode);
		end = steady_clock::now();

		if(winlirc_decodeCommand(&hw,remotes,out,out_size)) {
			ResetEvent(dataReadyEvent);
			return 1;
		}

		ResetEvent(dataReadyEvent);
	}

	return 0;
}

WL_API hardware const* getHardware() {

	initHardwareStruct();
	return &hw;
}
