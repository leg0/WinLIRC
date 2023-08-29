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
extern hardware const geniatech_hw;
extern rbuf rec_buffer;

static int geniatech_init(winlirc_api const* winlirc) {

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);

	receive = new Receive();
	return receive->init();
}

static void geniatech_deinit() {

	if(receive) {
		receive->deinit();
		delete receive;
		receive = NULL;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = NULL;
}

static int geniatech_hasGui() {

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

static void	geniatech_loadSetupGui() {

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

static int geniatech_sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return 0;
}

static int geniatech_decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(receive)
	{
		if(!receive->waitTillDataIsReady(0)) {
			return 0;
		}

		::last = end;
		start = steady_clock::now();
		receive->getData(&irCode);
		end = steady_clock::now();

		if(winlirc_decodeCommand(&rec_buffer, &geniatech_hw,remotes,out,out_size)) {
			ResetEvent(dataReadyEvent);
			return 1;
		}

		ResetEvent(dataReadyEvent);
	}

	return 0;
}

static hardware const* geniatech_getHardware() {

	return &geniatech_hw;
}

WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = geniatech_init,
		.deinit = geniatech_deinit,
		.hasGui = geniatech_hasGui,
		.loadSetupGui = geniatech_loadSetupGui,
		.sendIR = geniatech_sendIR,
		.decodeIR = geniatech_decodeIR,
		.getHardware = geniatech_getHardware,
		.hardware = &geniatech_hw,
	};
	return &p;
}
