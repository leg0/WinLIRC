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
#include "Iguana.h"
#include "resource.h"
#include "Settings.h"
#include "Globals.h"
#include "Decode.h"
#include "LIRCDefines.h"
#include "iguanaIR.h"
#include <stdio.h>
#include "ReceiveData.h"
#include "hardware.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

IG_API int init(HANDLE exitEvent) {

	init_rec_buffer();
	initHardwareStruct();

	receiveData = new ReceiveData();

	if(!receiveData->init()) return 0;

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);

	return 1;
}

IG_API void deinit() {

	if(receiveData) {
		receiveData->deinit();
		delete receiveData;
		receiveData = NULL;
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

			SetDlgItemInt(hwnd,IDC_EDIT1,settings.getDeviceNumber(),FALSE);

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

					if(temp<0) temp = 0;

					settings.setDeviceNumber(temp);
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

IG_API void	loadSetupGui() {

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

IG_API int sendIR(struct ir_remote *remotes, struct ir_ncode *code, int repeats) {

	//
	// return false - since we don't support this function yet .. Tira should be able to send though
	//

	return 0;
}

IG_API int decodeIR(struct ir_remote *remotes, char *out) {

	if(receiveData) {
		receiveData->waitTillDataIsReady(0);
	}

	if(decodeCommand(remotes,out)) {
		return 1;
	}

	return 0;
}

IG_API struct hardware* getHardware() {

	initHardwareStruct();
	return &hw;

}