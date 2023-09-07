#include "TechnotrendPlugin.h"
#include "Globals.h"

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include "../Common/Win32Helpers.h"

#include "Globals.h"
#include "resource.h"
#include "Settings.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
extern hardware const technotrend_hw;
extern rbuf rec_buffer;

static int technotrend_init(plugin_interface* self, winlirc_api const* winlirc)
{
	winlirc_init_rec_buffer(&rec_buffer);

	threadExitEvent = reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc));
	dataReadyEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	auto receive = static_cast<TechnotrendPlugin*>(self);
	return receive->init(settings.getDeviceNumber(), settings.getBusyLED(), settings.getPowerLED());
}

static void technotrend_deinit(plugin_interface* self)
{
	auto receive = static_cast<TechnotrendPlugin*>(self);
	if (receive)
	{
		receive->deinit();
		delete receive;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;
}

static int technotrend_hasGui(plugin_interface* self)
{
	return TRUE;
}

static INT_PTR CALLBACK dialogProc(HWND hwnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam) {

	switch (message) {

	case WM_INITDIALOG: {

		//======================
		BYTE	numberOfDevices;
		TCHAR	temp[4];
		//======================

		numberOfDevices = irGetNumDevices();

		_stprintf(temp, _T("%i"), 0);
		SendDlgItemMessage(hwnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)temp);

		for (int i = 1; i < numberOfDevices; i++) {
			_stprintf(temp, _T("%i"), i);
			SendDlgItemMessage(hwnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)temp);
		}

		SendDlgItemMessage(hwnd, IDC_COMBO1, CB_SETCURSEL, settings.getDeviceNumber(), 0);

		if (settings.getBusyLED())	SendDlgItemMessage(hwnd, IDC_CHECK1, BM_SETCHECK, BST_CHECKED, 0);
		if (settings.getPowerLED())	SendDlgItemMessage(hwnd, IDC_CHECK2, BM_SETCHECK, BST_CHECKED, 0);

		ShowWindow(hwnd, SW_SHOW);

		return TRUE;
	}

	case WM_COMMAND: {

		switch (LOWORD(wParam)) {

		case IDOK: {

			if (SendDlgItemMessage(hwnd, IDC_CHECK1, BM_GETSTATE, 0, 0) == BST_CHECKED) {
				settings.setBusyLED(TRUE);
			}
			else {
				settings.setBusyLED(FALSE);
			}

			if (SendDlgItemMessage(hwnd, IDC_CHECK2, BM_GETSTATE, 0, 0) == BST_CHECKED) {
				settings.setPowerLED(TRUE);
			}
			else {
				settings.setPowerLED(FALSE);
			}

			settings.setDeviceNumber(SendDlgItemMessage(hwnd, IDC_COMBO1, CB_GETCURSEL, 0, 0));

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

static void	technotrend_loadSetupGui(plugin_interface* self) {

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

static int technotrend_sendIR(plugin_interface* self, ir_remote* remote, ir_ncode* code, int repeats)
{
	return 0;
}

static int technotrend_decodeIR(plugin_interface* self, ir_remote* remotes, char* out, size_t out_size) {

	//wait till data is ready
	auto receive = static_cast<TechnotrendPlugin*>(self);

	if (receive) {
		if (!receive->waitTillDataIsReady(0)) {
			return false;
		}
	}

	winlirc_clear_rec_buffer(&rec_buffer, &technotrend_hw);

	if (winlirc_decodeCommand(&rec_buffer, &technotrend_hw, remotes, out, out_size)) {
		return 1;
	}

	return 0;
}

static hardware const* technotrend_getHardware(plugin_interface const* self)
{
	return &technotrend_hw;
}

WL_API plugin_interface* getPluginInterface()
{
	return new TechnotrendPlugin();
}

TechnotrendPlugin::TechnotrendPlugin()
	: plugin_interface{
		.plugin_api_version = winlirc_plugin_api_version,
		.init = technotrend_init,
		.deinit = technotrend_deinit,
		.hasGui = technotrend_hasGui,
		.loadSetupGui = technotrend_loadSetupGui,
		.sendIR = technotrend_sendIR,
		.decodeIR = technotrend_decodeIR,
		.getHardware = technotrend_getHardware,
		.hardware = &technotrend_hw,
	}
{ }

TechnotrendPlugin::~TechnotrendPlugin()
{
	deinit();
}

void callBackFunc(PVOID Context, PVOID Buf, ULONG len, USBIR_MODES IRMode, HANDLE hOpen, BYTE DevIdx) {

	if(Context != nullptr) {
		((TechnotrendPlugin*)Context)->callBackFunction(Buf, len, IRMode, hOpen, DevIdx);
	}
}

int TechnotrendPlugin::init(int deviceID, int busyLED, int powerLED) {

	deviceHandle = irOpen(deviceID, USBIR_MODE_DIV, &callBackFunc, this);

	if(deviceHandle==INVALID_HANDLE_VALUE) return 0;

	if(busyLED)	irSetBusyLEDFreq(deviceHandle, 128);
	else		irSetBusyLEDFreq(deviceHandle, 0);

	if(powerLED)irSetPowerLED	(deviceHandle, TRUE);
	else		irSetPowerLED	(deviceHandle, FALSE);

	return 1;
}

void TechnotrendPlugin::deinit() {

	if(deviceHandle!=INVALID_HANDLE_VALUE) {

		irSetBusyLEDFreq(deviceHandle, FALSE);
		irSetPowerLED	(deviceHandle, FALSE);
		irClose			(deviceHandle);

		deviceHandle = INVALID_HANDLE_VALUE;
	}
}

void TechnotrendPlugin::setData(lirc_t data) {

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool TechnotrendPlugin::dataReady() {

	if(bufferStart==bufferEnd) return false;
	
	return true;
}

bool TechnotrendPlugin::getData(lirc_t *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

void TechnotrendPlugin::callBackFunction(PVOID Buf, ULONG len, USBIR_MODES IRMode, HANDLE hOpen, BYTE DevIdx) {

	//=============
	lirc_t *buffer;
	//=============

	//
	// santity checks
	//
	if(hOpen==INVALID_HANDLE_VALUE) return;
	if(IRMode!=USBIR_MODE_DIV)		return;

	buffer = (lirc_t*)Buf;

	if(len>256) len = 256;		// to fit our buffer but if it sends data too infrequently will be severe key press lag anyway 

	for(unsigned int i=0; i<len; i++) {
		setData(buffer[i]);		// no conversion needed (hopefully)
	}

	SetEvent(dataReadyEvent);
}

bool TechnotrendPlugin::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==nullptr) evt=1;
	else evt=2;

	if(!dataReady())
	{
		ResetEvent(dataReadyEvent);
		int res;
		if(maxUSecs)
			res=WaitForMultipleObjects(evt,events,FALSE,(maxUSecs+500)/1000);
		else
			res=WaitForMultipleObjects(evt,events,FALSE,INFINITE);
		if(res==(WAIT_OBJECT_0+1))
		{
			return false;
		}
	}

	return true;
}