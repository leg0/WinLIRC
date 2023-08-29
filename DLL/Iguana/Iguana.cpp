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
#include "resource.h"
#include "Settings.h"
#include "Globals.h"
#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include "../Common/Win32Helpers.h"
#include "iguanaIR.h"
#include "SendReceiveData.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
extern hardware const iguana_hw;
extern rbuf rec_buffer;
extern sbuf send_buffer;

static int iguana_init(winlirc_api const* winlirc) {

	winlirc_init_rec_buffer(&rec_buffer);
	winlirc_init_send_buffer(&send_buffer);

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);

	sendReceiveData = new SendReceiveData();

	if(!sendReceiveData->init()) return 0;
	if(!sendReceiveData->setTransmitters(settings.getTransmitterChannels())) return 0;

	return 1;
}

static void iguana_deinit() {

	if(sendReceiveData) {
		sendReceiveData->deinit();
		delete sendReceiveData;
		sendReceiveData = nullptr;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

static int iguana_hasGui() {

	return TRUE;
}

INT_PTR CALLBACK dialogProc (HWND hwnd, 
                             UINT message, 
                             WPARAM wParam, 
                             LPARAM lParam) {

    switch (message) {

		case WM_INITDIALOG: {
			
			//===============
			int transmitters;
			//===============

			transmitters = settings.getTransmitterChannels();

			if(transmitters&IGUANA_TRANSMITTER1) SendDlgItemMessage(hwnd,IDC_CHECK1,BM_SETCHECK,BST_CHECKED,0);
			if(transmitters&IGUANA_TRANSMITTER2) SendDlgItemMessage(hwnd,IDC_CHECK2,BM_SETCHECK,BST_CHECKED,0);
			if(transmitters&IGUANA_TRANSMITTER3) SendDlgItemMessage(hwnd,IDC_CHECK3,BM_SETCHECK,BST_CHECKED,0);
			if(transmitters&IGUANA_TRANSMITTER4) SendDlgItemMessage(hwnd,IDC_CHECK4,BM_SETCHECK,BST_CHECKED,0);

			SetDlgItemInt(hwnd,IDC_EDIT1,settings.getDeviceNumber(),FALSE);

			ShowWindow(hwnd, SW_SHOW);

			return TRUE;
		}

		case WM_COMMAND: {

			switch(LOWORD(wParam)) {

				case IDOK: {

					//===============
					int temp;
					int transmitters;
					//===============

					transmitters = 0;

					temp = GetDlgItemInt(hwnd,IDC_EDIT1,nullptr,FALSE);

					if(temp<0) temp = 0;

					settings.setDeviceNumber(temp);
					
					if(SendDlgItemMessage(hwnd,IDC_CHECK1,BM_GETSTATE,0,0)==BST_CHECKED) transmitters |= IGUANA_TRANSMITTER1;
					if(SendDlgItemMessage(hwnd,IDC_CHECK2,BM_GETSTATE,0,0)==BST_CHECKED) transmitters |= IGUANA_TRANSMITTER2;
					if(SendDlgItemMessage(hwnd,IDC_CHECK3,BM_GETSTATE,0,0)==BST_CHECKED) transmitters |= IGUANA_TRANSMITTER3;
					if(SendDlgItemMessage(hwnd,IDC_CHECK4,BM_GETSTATE,0,0)==BST_CHECKED) transmitters |= IGUANA_TRANSMITTER4;

					settings.setTransmitterChannels(transmitters);

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

static void	iguana_loadSetupGui() {

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

static int iguana_sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	if(sendReceiveData) {
		return sendReceiveData->send(remote,code,repeats);
	}

	return 0;
}

static int iguana_decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(sendReceiveData) {

		if(!sendReceiveData->waitTillDataIsReady(0)) {
			return 0;
		}

		winlirc_clear_rec_buffer(&rec_buffer, &iguana_hw);

		if(winlirc_decodeCommand(&rec_buffer, &iguana_hw,remotes,out,out_size)) {
			return 1;
		}
	}

	return 0;
}

static int iguana_setTransmitters(unsigned int transmitterMask) {

	if(sendReceiveData) {
		return sendReceiveData->setTransmitters(transmitterMask);
	}

	return 0;
}

static hardware const* iguana_getHardware() {

	return &iguana_hw;
}

WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = iguana_init,
		.deinit = iguana_deinit,
		.hasGui = iguana_hasGui,
		.loadSetupGui = iguana_loadSetupGui,
		.sendIR = iguana_sendIR,
		.decodeIR = iguana_decodeIR,
		.getHardware = iguana_getHardware,
		.hardware = &iguana_hw,
	};
	return &p;
}
