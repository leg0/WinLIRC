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

#include "MaxterPlusPlugin.h"

#include "Globals.h"
#include <tchar.h>
#include "../Common/Win32Helpers.h"
#include <winlirc/WLPluginAPI.h>
#include "resource.h"
#include <chrono>

using namespace std::chrono_literals;
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

static int maxterplus_init(plugin_interface* self, winlirc_api const* winlirc) {
	auto const sendReceiveData = static_cast<MaxterPlusPlugin*>(self);

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

	if (!sendReceiveData->init()) return 0;

	return 1;
}

static void maxterplus_deinit(plugin_interface* self) {
	auto const sendReceiveData = static_cast<MaxterPlusPlugin*>(self);

	if (sendReceiveData) {
		sendReceiveData->deinit();
		delete sendReceiveData;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

static int maxterplus_hasGui(plugin_interface* self) {

	return TRUE;
}

static INT_PTR CALLBACK dialogProc(HWND hwnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam) {

	switch (message) {

	case WM_INITDIALOG: {

		auto const buttonSettings = settings.getSettings();

		if (!(buttonSettings & 0x02)) { SendDlgItemMessage(hwnd, IDC_CHECK1, BM_SETCHECK, BST_CHECKED, 0); }
		if (!(buttonSettings & 0x04)) { SendDlgItemMessage(hwnd, IDC_CHECK2, BM_SETCHECK, BST_CHECKED, 0); }
		if (!(buttonSettings & 0x08)) { SendDlgItemMessage(hwnd, IDC_CHECK3, BM_SETCHECK, BST_CHECKED, 0); }
		if (!(buttonSettings & 0x10)) { SendDlgItemMessage(hwnd, IDC_CHECK4, BM_SETCHECK, BST_CHECKED, 0); }
		if (!(buttonSettings & 0x20)) { SendDlgItemMessage(hwnd, IDC_CHECK5, BM_SETCHECK, BST_CHECKED, 0); }

		ShowWindow(hwnd, SW_SHOW);

		return TRUE;
	}

	case WM_COMMAND: {

		switch (LOWORD(wParam)) {

		case IDOK: {

			int buttonSettings = 0;

			if (SendDlgItemMessage(hwnd, IDC_CHECK1, BM_GETSTATE, 0, 0) == BST_UNCHECKED) { buttonSettings = buttonSettings | 0x02; }
			if (SendDlgItemMessage(hwnd, IDC_CHECK2, BM_GETSTATE, 0, 0) == BST_UNCHECKED) { buttonSettings = buttonSettings | 0x04; }
			if (SendDlgItemMessage(hwnd, IDC_CHECK3, BM_GETSTATE, 0, 0) == BST_UNCHECKED) { buttonSettings = buttonSettings | 0x08; }
			if (SendDlgItemMessage(hwnd, IDC_CHECK4, BM_GETSTATE, 0, 0) == BST_UNCHECKED) { buttonSettings = buttonSettings | 0x10; }
			if (SendDlgItemMessage(hwnd, IDC_CHECK5, BM_GETSTATE, 0, 0) == BST_UNCHECKED) { buttonSettings = buttonSettings | 0x20; }

			settings.setSettings(buttonSettings);
			settings.saveSettings();

			DestroyWindow(hwnd);
			return TRUE;
		}

		case IDCANCEL: {
			//
			//ignore changes
			//
			DestroyWindow(hwnd);
			return TRUE;
		}

		}

		return FALSE;

	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return TRUE;
	}

	return FALSE;

}

static void	maxterplus_loadSetupGui(plugin_interface* self) {

	//==============
	HWND	hDialog;
	MSG		msg;
	INT		status;
	//==============

	hDialog = CreateDialog((HINSTANCE)(&__ImageBase), MAKEINTRESOURCE(IDD_DIALOG1), nullptr, dialogProc);

	while ((status = GetMessage(&msg, 0, 0, 0)) != 0) {

		if (status == -1) return;

		if (!IsDialogMessage(hDialog, &msg)) {

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

static int maxterplus_sendIR(plugin_interface* self, ir_remote* remote, ir_ncode* code, int repeats) {

	return 0;
}

static int maxterplus_decodeIR(plugin_interface* self, ir_remote* remotes, char* out, size_t out_size) {
	auto const sendReceiveData = static_cast<MaxterPlusPlugin*>(self);
	if (sendReceiveData) {

		if (!sendReceiveData->waitTillDataIsReady(0us)) {
			return 0;
		}

		return sendReceiveData->decodeCommand(out, out_size);
	}

	return 0;
}

MaxterPlusPlugin::MaxterPlusPlugin() noexcept
	: plugin_interface{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = maxterplus_init,
		.deinit = maxterplus_deinit,
		.hasGui = maxterplus_hasGui,
		.loadSetupGui = maxterplus_loadSetupGui,
		.sendIR = maxterplus_sendIR,
		.decodeIR = maxterplus_decodeIR,
		.getHardware = [](plugin_interface const*) -> struct hardware const* { return nullptr; },
		.hardware = nullptr,
	}
{
}

bool MaxterPlusPlugin::init() {

	//==========================
	PHID_DEVICE	devices;
	ULONG		numberOfDevices;
	BOOL		deviceFound;
	TCHAR		deviceName[1024];
	//==========================

	deviceFound = 0;
	toggleBit	= FALSE;
	irCode		= 0;
	lastValue	= 0;
	repeats		= 0;

	if(FindKnownHidDevices(&devices,&numberOfDevices)) {

		for(UINT i=0; i<numberOfDevices; i++) {

			if(devices[i].Attributes.ProductID==0x37) {
				if(devices[i].Attributes.VendorID==0x18B1) {
					if(devices[i].Caps.UsagePage==0xFF80) {
						if(devices[i].Caps.Usage==0) {
							_tcscpy_s(deviceName,_countof(deviceName),devices[i].DevicePath);
							deviceFound = TRUE;
						}
					}
				}
			}
		}
		CloseHidDevices(devices,numberOfDevices);
		free(devices);
	}

	if(deviceFound) {

		if(OpenHidDevice(deviceName,TRUE,TRUE,TRUE,FALSE,&device)) {

			threadHandle = std::jthread{ [this](std::stop_token stop) { this->threadProc(stop); } };
			return true;
		}
	}

	return false;
}

void MaxterPlusPlugin::deinit() {

	threadHandle.request_stop();
	::SetEvent(exitEvent);
}

void MaxterPlusPlugin::threadProc(std::stop_token stop) {

	OVERLAPPED	overlappedRead = { 0 };
	overlappedRead.hEvent = CreateEvent(nullptr,FALSE,FALSE,nullptr);
	exitEvent = CreateEvent(nullptr,TRUE,FALSE,nullptr);

	HANDLE const events[] = { overlappedRead.hEvent, exitEvent };

	setFeatures();

	while(!stop.stop_requested()) {

		UCHAR buffer[5];
		DWORD bytesRead;
		if(!ReadFile(device.HidDevice,buffer,5,&bytesRead,&overlappedRead)) {
			//break;
		}

		auto const result = WaitForMultipleObjects(2,events,FALSE,INFINITE);

		if(result==(WAIT_OBJECT_0)) 
		{
			irCode = buffer[2];

			if(irCode==lastValue) {
				repeats++;
			}
			else {
				repeats = 0;
			}

			//printf("irCode %i repeats %i\n",irCode,repeats);

			SetEvent(dataReadyEvent);
		}

		if(result==(WAIT_OBJECT_0+1))
		{
			//printf("leaving thread \n");
			break;
		}
	}

	restoreFeatures();

	CloseHidDevice(&device);

	SAFE_CLOSE_HANDLE(exitEvent);
	SAFE_CLOSE_HANDLE(overlappedRead.hEvent);
}

bool MaxterPlusPlugin::waitTillDataIsReady(std::chrono::microseconds maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	DWORD const evt = (threadExitEvent==nullptr) ? 1 : 2;

	if(irCode==0)
	{
		ResetEvent(dataReadyEvent);
		using namespace std::chrono;
		DWORD const dwTimeout = maxUSecs > 0us
			? duration_cast<milliseconds>(maxUSecs + 500us).count()
			: INFINITE;
		DWORD const res = ::WaitForMultipleObjects(evt, events, false, dwTimeout);
		if(res==(WAIT_OBJECT_0+1))
		{
			return false;
		}
	}

	return true;
}

BOOL MaxterPlusPlugin::setFeatures() {

	device.FeatureReportBuffer[0] = 6;
	device.FeatureReportBuffer[1] = (CHAR)0xFF;	//to shut the compiler up
	device.FeatureReportBuffer[2] = (CHAR)settings.getSettings();
	device.FeatureReportBuffer[3] = 0;
	device.FeatureReportBuffer[4] = 0;

	return HidD_SetFeature (device.HidDevice, device.FeatureReportBuffer, device.Caps.FeatureReportByteLength);

}

void MaxterPlusPlugin::restoreFeatures() {

	device.FeatureReportBuffer[0] = 6;
	device.FeatureReportBuffer[1] = (CHAR)0xFF;	//to shut the compiler up
	device.FeatureReportBuffer[2] = 0x40;
	device.FeatureReportBuffer[3] = 0;
	device.FeatureReportBuffer[4] = 0;

	HidD_SetFeature (device.HidDevice, device.FeatureReportBuffer, device.Caps.FeatureReportByteLength);
}

int MaxterPlusPlugin::decodeCommand(char *out, size_t out_size) {

	//==================
	UINT outCode;
	char buttonName[32];
	//==================

	outCode = irCode & 0x7F;

	switch(outCode) {
		
		case 12:	strcpy_s(buttonName,"POWER");	break;

		case 63:	strcpy_s(buttonName,"0");	break;
		case 1:		strcpy_s(buttonName,"1");	break;
		case 2:		strcpy_s(buttonName,"2");	break;
		case 3:		strcpy_s(buttonName,"3");	break;
		case 4:		strcpy_s(buttonName,"4");	break;
		case 5:		strcpy_s(buttonName,"5");	break;
		case 6:		strcpy_s(buttonName,"6");	break;
		case 7:		strcpy_s(buttonName,"7");	break;
		case 8:		strcpy_s(buttonName,"8");	break;
		case 9:		strcpy_s(buttonName,"9");	break;

		case 40:	strcpy_s(buttonName,"REC_TV");	break;
		case 38:	strcpy_s(buttonName,"GUIDE");	break;
		case 37:	strcpy_s(buttonName,"LIVE_TV");break;

		case 35:	strcpy_s(buttonName,"BACK");	break;
		case 15:	strcpy_s(buttonName,"MORE");	break;

		case 30:	strcpy_s(buttonName,"UP");		break;
		case 31:	strcpy_s(buttonName,"DOWN");	break;
		case 32:	strcpy_s(buttonName,"LEFT");	break;
		case 33:	strcpy_s(buttonName,"RIGHT");	break;
		case 34:	strcpy_s(buttonName,"OK");		break;

		case 17:	strcpy_s(buttonName,"VOL-");	break;
		case 16:	strcpy_s(buttonName,"VOL+");	break;
		case 19:	strcpy_s(buttonName,"CH/PG-");	break;
		case 18:	strcpy_s(buttonName,"CH/PG+");	break;

		case 23:	strcpy_s(buttonName,"RECORD");	break;
		case 13:	strcpy_s(buttonName,"MCE");	break;
		case 25:	strcpy_s(buttonName,"STOP");	break;
		case 11:	strcpy_s(buttonName,"MENU");	break;

		case 21:	strcpy_s(buttonName,"<<");		break;
		case 24:	strcpy_s(buttonName,"||");		break;
		case 20:	strcpy_s(buttonName,">>");		break;
		case 10:	strcpy_s(buttonName,"ESC");	break;

		case 27:	strcpy_s(buttonName,"|<<");	break;
		case 22:	strcpy_s(buttonName,"PLAY");	break;
		case 26:	strcpy_s(buttonName,">>|");	break;
		case 14:	strcpy_s(buttonName,"MUTE");	break;

		case 93:	strcpy_s(buttonName,"*");		break;
		case 74:	strcpy_s(buttonName,"CLEAR");	break;
		case 92:	strcpy_s(buttonName,"#");		break;

		case 98:	strcpy_s(buttonName,"MOUSE_BUTTON_LEFT");	break;
		case 79:	strcpy_s(buttonName,"MOUSE_BUTTON_RIGHT");break;

		case 94:	strcpy_s(buttonName,"MOUSE_UP");	break;
		case 95:	strcpy_s(buttonName,"MOUSE_DOWN");	break;
		case 96:	strcpy_s(buttonName,"MOUSE_LEFT");	break;
		case 97:	strcpy_s(buttonName,"MOUSE_RIGHT");break;

		case 100:	strcpy_s(buttonName,"HELP");		break;
		case 36:	strcpy_s(buttonName,"DVD_MENU");	break;
		case 99:	strcpy_s(buttonName,"FULLSCREEN");	break;
		case 106:	strcpy_s(buttonName,"ENTER");		break;

		default: {
			irCode	= 0;
			repeats = 0;
			return 0;
		}

	}

	lastValue	= irCode;
	irCode		= 0;

	_snprintf_s(out, out_size, out_size,"%016llx %02x %s %s\n",__int64(0),repeats,buttonName,"MaxterPlus");

	return 1;

}



WL_API plugin_interface* getPluginInterface() {
	return new MaxterPlusPlugin();
}
