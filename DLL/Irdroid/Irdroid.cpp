/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * Irdroid is a OSHW module based on DP IRToy
 * Copyright (C) 2010 Ian Curtis
 */

#include <Windows.h>
#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include "../Common/enumSerialPorts.h"
#include "../Common/Win32Helpers.h"

#include "Globals.h"
#include "resource.h"
#include <tchar.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
extern hardware const irdroid_hw;
extern rbuf rec_buffer;

static int irdroid_init(winlirc_api const* winlirc) {

	winlirc_init_rec_buffer(&rec_buffer);

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);

	sendReceiveData = new SendReceiveData();

	if(!sendReceiveData->init()) return 0;

	return 1;
}

static void irdroid_deinit() {

	if(sendReceiveData) {
		sendReceiveData->deinit();
		delete sendReceiveData;
		sendReceiveData = nullptr;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

static int irdroid_hasGui() {

	return TRUE;
}

INT_PTR CALLBACK dialogProc (HWND hwnd, 
                          UINT message, 
                          WPARAM wParam, 
                          LPARAM lParam) {

    switch (message) {

		case WM_INITDIALOG: {

			enumSerialPorts(hwnd, IDC_COMBO1);

			TCHAR portName[32];
			_stprintf(portName, _T("COM%d"), settings.getComPort());
			int const portIdx = SendDlgItemMessage(hwnd, IDC_COMBO1, CB_FINDSTRINGEXACT, -1, reinterpret_cast<LPARAM>(portName));
			SendDlgItemMessage(hwnd, IDC_COMBO1, CB_SETCURSEL, portIdx, 0);

			ShowWindow(hwnd, SW_SHOW);

			return TRUE;
		}

		case WM_COMMAND: {

			switch(LOWORD(wParam)) {

				case IDOK: {

					int const curSel = SendDlgItemMessage(hwnd, IDC_COMBO1, CB_GETCURSEL, 0, 0);
					if (curSel != CB_ERR)
					{
						int const textLen = SendDlgItemMessage(hwnd, IDC_COMBO1, CB_GETLBTEXTLEN, curSel, 0);
						std::vector<TCHAR> text(textLen + 1);
						SendDlgItemMessage(hwnd, IDC_COMBO1, CB_GETLBTEXT, curSel, reinterpret_cast<LPARAM>(&text[0]));
						if (text.size() > 3) // should start with "COM"
						{
							settings.setComPort(_tstoi(&text[3]));
							settings.saveSettings();
						}
					}

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

static void	irdroid_loadSetupGui() {

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

static int irdroid_sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	if(sendReceiveData) {
		return sendReceiveData->send(remote,code,repeats);
	}

	return 0;
}

static int irdroid_decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(sendReceiveData) {

		using namespace std::chrono_literals;
		if(!sendReceiveData->waitTillDataIsReady(0us)) {
			return 0;
		}

		winlirc_clear_rec_buffer(&rec_buffer, &irdroid_hw);

		if(winlirc_decodeCommand(&rec_buffer, &irdroid_hw,remotes,out,out_size)) {
			return 1;
		}
	}

	return 0;
}

static hardware const* irdroid_getHardware() {

	return &irdroid_hw;
}

WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = irdroid_init,
		.deinit = irdroid_deinit,
		.hasGui = irdroid_hasGui,
		.loadSetupGui = irdroid_loadSetupGui,
		.sendIR = irdroid_sendIR,
		.decodeIR = irdroid_decodeIR,
		.getHardware = irdroid_getHardware,
		.hardware = &irdroid_hw,
	};
	return &p;
}
