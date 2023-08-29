#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include "Globals.h"
#include "resource.h"
#include "Receive.h"
#include "Settings.h"
#include <winlirc/WLPluginAPI.h>
#include <winlirc/winlirc_api.h>
#include "../Common/Win32helpers.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
extern hardware const technisat_hw;
extern rbuf rec_buffer;

static int technisat_init(winlirc_api const* winlirc) {

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent	= CreateEvent(nullptr,FALSE,FALSE,nullptr);

	receive = new Receive();
	return receive->init(settings.getDeviceNumber());
}

static void technisat_deinit() {

	if(receive) {
		receive->deinit();
		delete receive;
		receive = nullptr;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

static int technisat_hasGui() {

	return TRUE;
}

INT_PTR CALLBACK dialogProc(HWND hwnd, 
                          UINT message, 
                          WPARAM wParam, 
                          LPARAM lParam) {

    switch (message) {

		case WM_INITDIALOG: {

			//======================
			BYTE	numberOfDevices;
			TCHAR	temp[4];
			//======================
			if (!receive)
			{
				receive = new Receive();
				receive->init(6969);
			}
			numberOfDevices = receive->enumDevices();

			SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_RESETCONTENT,0,0);

			_stprintf_s(temp,_T("%i"),0);
			SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_ADDSTRING,0,(LPARAM)temp);

			for(int i=1; i<numberOfDevices; i++) {
				_stprintf_s(temp,_T("%i"),i);
				SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_ADDSTRING,0,(LPARAM)temp);
			}

			if (settings.getDeviceNumber()>=numberOfDevices)
				settings.setDeviceNumber(0);

			SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_SETCURSEL,settings.getDeviceNumber(),0);

			ShowWindow( GetDlgItem(hwnd,IDC_COMBO_DEVID), numberOfDevices ? SW_SHOW : SW_HIDE);

			ShowWindow(hwnd, SW_SHOW);

			return TRUE;
		}

		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK: {
					settings.setDeviceNumber(SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_GETCURSEL,0,0));
					settings.saveSettings();
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

static void technisat_loadSetupGui() {

	//==============
	HWND	hDialog;
	MSG		msg;
    INT		status;
	//==============

	hDialog = CreateDialog((HINSTANCE)(&__ImageBase),MAKEINTRESOURCE(IDD_DIALOG_CFG),nullptr,dialogProc);

    while ((status = GetMessage (& msg, 0, 0, 0)) != 0) {

        if (status == -1) return;

        if (!IsDialogMessage (hDialog, & msg)) {

            TranslateMessage ( & msg );
            DispatchMessage ( & msg );
        }
    }

}

static int technisat_sendIR(struct ir_remote* remote, struct ir_ncode* code, int repeats) {

	return 0;
}

static int technisat_decodeIR(struct ir_remote* remotes, char* out, size_t out_size) {

	//wait till data is ready

	if (receive)
	{
		using namespace std::chrono_literals;
		if (!receive->waitTillDataIsReady(0us)) {
			return 0;
		}

		last = end;
		start = std::chrono::steady_clock::now();
		receive->getData(&irCode);
		end = std::chrono::steady_clock::now();

		if (winlirc_decodeCommand(&rec_buffer, &technisat_hw, remotes, out, out_size)) {
			ResetEvent(dataReadyEvent);
			return 1;
		}

		ResetEvent(dataReadyEvent);
	}

	return 0;
}

static hardware const* technisat_getHardware() {
	return &technisat_hw;
}

WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = technisat_init,
		.deinit = technisat_deinit,
		.hasGui = technisat_hasGui,
		.loadSetupGui = technisat_loadSetupGui,
		.sendIR = technisat_sendIR,
		.decodeIR = technisat_decodeIR,
		.getHardware = technisat_getHardware,
		.hardware = &technisat_hw,
	};
	return &p;
}
