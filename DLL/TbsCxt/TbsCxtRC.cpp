#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#include "Globals.h"
#include "resource.h"
#include "Receive.h"
#include "Settings.h"

#include <winlirc/WLPluginAPI.h>
#include <winlirc/IRRemote.h>
#include "../Common/Win32Helpers.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
void initHardwareStruct();
extern hardware hw;

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

INT_PTR CALLBACK dialogProc (HWND hwnd, 
                             UINT message, 
                             WPARAM wParam, 
                             LPARAM lParam)
{
	USES_CONVERSION;	
    switch (message) {

		case WM_INITDIALOG: {
			CoInitializeEx(nullptr,COINIT_MULTITHREADED);
			{
				int numberOfDevices=0;
				SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_RESETCONTENT,0,0);

				// create system device enumerator
				CComPtr <ICreateDevEnum>	pSysDevEnum = nullptr;	
				HRESULT hr = pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
				if (hr == S_OK)
				{
					// create a class enumerator for the desired category defined by classGuid.
					CComPtr <IEnumMoniker> pEnumCat = nullptr;	//moniker enumerator for filter categories
					hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
					if (hr == S_OK)
					{

						// reset the enumeration
						pEnumCat->Reset();

						// now iterate through enumeration
						ULONG cFetched = 0;
						CComPtr <IMoniker> pMoniker = nullptr;

						// get a pointer to each moniker
						while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
						{
							//get a pointer to property bag (which has filter)
							IPropertyBag* pPropBag = nullptr;	
							if (pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag) != S_OK)
								break;

							char szFriendlyName[MAX_PATH];

							CComPtr <IBaseFilter> pFilter = nullptr;
							// create an instance of the filter
							hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter);
							if (hr == S_OK)
							{							
								VARIANT varName;
								// retrieve the friendly name of the filter
								VariantInit(&varName);
								hr = pPropBag->Read(L"FriendlyName", &varName, 0);
								WideCharToMultiByte(CP_ACP, 0, varName.bstrVal, -1, szFriendlyName, sizeof(szFriendlyName), 0, 0);
								VariantClear(&varName);

								CComPtr <IKsPropertySet> pKsVCPropSet = nullptr;
								// query for interface
								hr = pFilter->QueryInterface(IID_IKsPropertySet, (void **)&pKsVCPropSet);
								if (pKsVCPropSet)
								{
									DWORD type_support;
									hr = pKsVCPropSet->QuerySupported(KSPROPSETID_CustomIRCaptureProperties,
										KSPROPERTY_IRCAPTURE_COMMAND,
										&type_support);
									if ( SUCCEEDED(hr) && (type_support & KSPROPERTY_SUPPORT_SET) )
									{
										PCHAR t=strstr(szFriendlyName,"Video Capture");
										if (t) *t=0;
										SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_ADDSTRING,0,(LPARAM)szFriendlyName);
										numberOfDevices++;
									}
								}
							}
							pMoniker.Release();
						}
					}
				}
			
				if (settings.getDeviceNumber()>=numberOfDevices)
					settings.setDeviceNumber(0);

				if (numberOfDevices == 0)
					SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_ADDSTRING,0,(LPARAM)"NO DEVICE");

				SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_SETCURSEL,settings.getDeviceNumber(),0);

				ShowWindow(hwnd, SW_SHOW);
			}

			CoUninitialize ();

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

WL_API int decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

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

		if(winlirc_decodeCommand(&hw,remotes,out,out_size)) {
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
