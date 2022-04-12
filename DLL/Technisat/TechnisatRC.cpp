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
void initHardwareStruct();
extern hardware hw;
extern rbuf rec_buffer;

WL_API int init(WLEventHandle exitEvent) {

	initHardwareStruct();

	threadExitEvent = reinterpret_cast<HANDLE>(exitEvent);
	dataReadyEvent	= CreateEvent(nullptr,FALSE,FALSE,nullptr);

	receive = new Receive();
	return receive->init(settings.getDeviceNumber());
}

WL_API void deinit() {

	if(receive) {
		receive->deinit();
		delete receive;
		receive = nullptr;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

WL_API int hasGui() {

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

WL_API void	loadSetupGui() {

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

WL_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return 0;
}

WL_API int decodeIR(struct ir_remote *remotes, size_t remotes_count, char *out, size_t out_size) {

	//wait till data is ready

	if(receive)
	{
		using namespace std::chrono_literals;
		if(!receive->waitTillDataIsReady(0us)) {
			return 0;
		}

		last = end;
		start = std::chrono::steady_clock::now();
		receive->getData(&irCode);
		end = std::chrono::steady_clock::now();

		if(winlirc_decodeCommand(&rec_buffer, &hw,remotes,remotes_count,out,out_size)) {
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
