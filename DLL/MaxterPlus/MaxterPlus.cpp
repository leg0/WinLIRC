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
 *
 * Copyright (C) 2010 Ian Curtis
 */

#include <Windows.h>
#include <winlirc/WLPluginAPI.h>
#include "../Common/Win32Helpers.h"
#include <stdio.h>
#include "Globals.h"
#include <tchar.h>
#include "resource.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

WL_API int init(WLEventHandle exitEvent) {

	threadExitEvent = reinterpret_cast<HANDLE>(exitEvent);
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);

	sendReceiveData = new SendReceiveData();

	if(!sendReceiveData->init()) return 0;

	return 1;
}

WL_API void deinit() {

	if(sendReceiveData) {
		sendReceiveData->deinit();
		delete sendReceiveData;
		sendReceiveData = nullptr;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
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

			//=================
			int buttonSettings;
			//=================

			buttonSettings = settings.getSettings();

			if(!(buttonSettings&0x02)) { SendDlgItemMessage(hwnd,IDC_CHECK1,BM_SETCHECK,BST_CHECKED,0); }
			if(!(buttonSettings&0x04)) { SendDlgItemMessage(hwnd,IDC_CHECK2,BM_SETCHECK,BST_CHECKED,0); }
			if(!(buttonSettings&0x08)) { SendDlgItemMessage(hwnd,IDC_CHECK3,BM_SETCHECK,BST_CHECKED,0); }
			if(!(buttonSettings&0x10)) { SendDlgItemMessage(hwnd,IDC_CHECK4,BM_SETCHECK,BST_CHECKED,0); }
			if(!(buttonSettings&0x20)) { SendDlgItemMessage(hwnd,IDC_CHECK5,BM_SETCHECK,BST_CHECKED,0); }

			ShowWindow(hwnd, SW_SHOW);

			return TRUE;
		}

		case WM_COMMAND: {

			switch(LOWORD(wParam)) {

				case IDOK: {

					//=================
					int buttonSettings;
					//=================

					buttonSettings = 0;

					if(SendDlgItemMessage(hwnd,IDC_CHECK1,BM_GETSTATE,0,0)==BST_UNCHECKED) { buttonSettings = buttonSettings|0x02; }
					if(SendDlgItemMessage(hwnd,IDC_CHECK2,BM_GETSTATE,0,0)==BST_UNCHECKED) { buttonSettings = buttonSettings|0x04; }
					if(SendDlgItemMessage(hwnd,IDC_CHECK3,BM_GETSTATE,0,0)==BST_UNCHECKED) { buttonSettings = buttonSettings|0x08; }
					if(SendDlgItemMessage(hwnd,IDC_CHECK4,BM_GETSTATE,0,0)==BST_UNCHECKED) { buttonSettings = buttonSettings|0x10; }
					if(SendDlgItemMessage(hwnd,IDC_CHECK5,BM_GETSTATE,0,0)==BST_UNCHECKED) { buttonSettings = buttonSettings|0x20; }

					settings.setSettings(buttonSettings);
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

	hDialog = CreateDialog((HINSTANCE)(&__ImageBase),MAKEINTRESOURCE(IDD_DIALOG1),nullptr,dialogProc);

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

	if(sendReceiveData) {

		using namespace std::chrono_literals;
		if(!sendReceiveData->waitTillDataIsReady(0us)) {
			return 0;
		}

		return sendReceiveData->decodeCommand(out, out_size);
	}

	return 0;
}
