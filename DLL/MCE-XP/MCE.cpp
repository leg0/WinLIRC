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
#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include "../Common/Win32Helpers.h"
#include "resource.h"
#include "Globals.h"
#include "SendReceiveData.h"
#include "Registry.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
extern hardware const mcexp_hw;
rbuf rec_buffer;
static sbuf send_buffer;

static int mcexp_init(winlirc_api const* winlirc) {

	winlirc_init_rec_buffer(&rec_buffer);
	winlirc_init_send_buffer(&send_buffer);

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);

	sendReceiveData = new SendReceiveData();

	if(!sendReceiveData->init()) return 0;

	return 1;
}

static void mcexp_deinit() {

	if(sendReceiveData) {
		sendReceiveData->deinit();
		delete sendReceiveData;
		sendReceiveData = nullptr;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

static int mcexp_hasGui() {

	return TRUE;
}

INT_PTR CALLBACK dialogProc (HWND hwnd, 
                             UINT message, 
                             WPARAM wParam, 
                             LPARAM lParam) {

    switch (message) {

		case WM_INITDIALOG: {

			//===================
			BOOL	hidEnabled;
			INT		transmitters;
			//===================

			ShowWindow(hwnd, SW_SHOW);

			hidEnabled = RegistrySettings::hidEnabled();

			if(!hidEnabled) {
				SendDlgItemMessage(hwnd,IDC_CHECK3,BM_SETCHECK,BST_CHECKED,0);
			}

			transmitters = settings.getTransmitterChannels();

			if(transmitters&MCE_BLASTER_1) {
				SendDlgItemMessage(hwnd,IDC_CHECK1,BM_SETCHECK,BST_CHECKED,0);
			}

			if(transmitters&MCE_BLASTER_2) {
				SendDlgItemMessage(hwnd,IDC_CHECK2,BM_SETCHECK,BST_CHECKED,0);
			}

			return TRUE;
		}

		case WM_COMMAND: {

			switch(LOWORD(wParam)) {

				case IDOK: {

					//===================
					BOOL	success;
					INT		transmitters;
					//===================

					transmitters = 0;

					if(SendDlgItemMessage(hwnd,IDC_CHECK3,BM_GETSTATE,0,0)==BST_CHECKED) {
						success = RegistrySettings::setHIDState(FALSE);
					}
					else {
						success = RegistrySettings::setHIDState(TRUE);
					}

					if(SendDlgItemMessage(hwnd,IDC_CHECK1,BM_GETSTATE,0,0)==BST_CHECKED) {
						transmitters |= MCE_BLASTER_1;
					}

					if(SendDlgItemMessage(hwnd,IDC_CHECK2,BM_GETSTATE,0,0)==BST_CHECKED) {
						transmitters |= MCE_BLASTER_2;
					}

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

static void	mcexp_loadSetupGui() {

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

static int mcexp_sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	if(sendReceiveData) {
		return sendReceiveData->send(remote,code,repeats);
	}

	return 0;
}

static int mcexp_decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	if(sendReceiveData) {

		using namespace std::chrono_literals;
		if(!sendReceiveData->waitTillDataIsReady(0us)) {
			return 0;
		}

		winlirc_clear_rec_buffer(&rec_buffer, &mcexp_hw);

		if(winlirc_decodeCommand(&rec_buffer, &mcexp_hw,remotes,out,out_size)) {
			return 1;
		}
	}

	return 0;
}

static hardware const* mcexp_getHardware() {
	return &mcexp_hw;
}

WL_API plugin_interface const* getPluginInterface() {
	static constexpr plugin_interface p{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = mcexp_init,
		.deinit = mcexp_deinit,
		.hasGui = mcexp_hasGui,
		.loadSetupGui = mcexp_loadSetupGui,
		.sendIR = mcexp_sendIR,
		.decodeIR = mcexp_decodeIR,
		.getHardware = mcexp_getHardware,
		.hardware = &mcexp_hw,
	};
	return &p;
}
