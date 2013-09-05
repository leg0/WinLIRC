#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#include "Globals.h"
#include "resource.h"
#include "Receive.h"
#include "Settings.h"

#include "../Common/WLPluginAPI.h"
#include "../Common/Hardware.h"
#include "../Common/Linux.h"
#include "../Common/IRRemote.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

WL_API int init(HANDLE exitEvent) {
	initHardwareStruct();

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);

	receive = new Receive();
	return receive->init(settings.getDeviceNumber(),(tmIrDecoderId)settings.getRcType());
}

WL_API void deinit() {

	if(receive) {
		receive->deinit();
		delete receive;
		receive = NULL;
	}

	if(dataReadyEvent) {
		CloseHandle(dataReadyEvent);
		dataReadyEvent = NULL;
	}

	threadExitEvent = NULL;	
}

WL_API int hasGui() {

	return TRUE;
}

BOOL CALLBACK dialogProc (HWND hwnd, 
                          UINT message, 
                          WPARAM wParam, 
                          LPARAM lParam)
{
	USES_CONVERSION;	
    switch (message) {

		case WM_INITDIALOG: {
			CoInitialize(NULL);
			int numberOfDevices=0;
			SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_RESETCONTENT,0,0);			

			// create system device enumerator
			CComPtr <ICreateDevEnum>	pSysDevEnum = NULL;	
			HRESULT hr = pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
			if (hr == S_OK)
			{
				// create a class enumerator for the desired category defined by classGuid.
				CComPtr <IEnumMoniker> pEnumCat = NULL;	//moniker enumerator for filter categories
				hr = pSysDevEnum->CreateClassEnumerator(CLSID_DeviceControlCategory, &pEnumCat, 0);
				if (hr == S_OK)
				{

					// reset the enumeration
					pEnumCat->Reset();

					// now iterate through enumeration
					ULONG cFetched = 0;
					CComPtr <IMoniker> pMoniker = NULL;

					// get a pointer to each moniker
					while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
					{
						//get a pointer to property bag (which has filter)
						CComPtr <IPropertyBag> pPropBag = NULL;	
						if (pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag) != S_OK)
							break;

						TCHAR szFriendlyName[MAX_PATH];
						VARIANT varName;
						// retrieve the friendly name of the filter
						VariantInit(&varName);
						hr = pPropBag->Read(L"FriendlyName", &varName, 0);
						WideCharToMultiByte(CP_ACP, 0, varName.bstrVal, -1, szFriendlyName, sizeof(szFriendlyName), 0, 0);
						VariantClear(&varName);

						PCHAR t=strstr(szFriendlyName,"BDA Main Device");
						if (t) *t=0;
						if ( strstr(szFriendlyName,"TBS") || strstr(szFriendlyName,"TT-budget") )
						{
							SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_ADDSTRING,0,(LPARAM)szFriendlyName);
							numberOfDevices++;
						}
						pMoniker.Release();
					}
				}
			}
			
			if (settings.getDeviceNumber() >= numberOfDevices)
				settings.setDeviceNumber(0);

			if (numberOfDevices == 0)
				SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_ADDSTRING,0,(LPARAM)"NO DEVICE");

			SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_RESETCONTENT,0,0);
			SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_ADDSTRING,0,(LPARAM)"RC5");
			SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_ADDSTRING,0,(LPARAM)"RC6");
			SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_ADDSTRING,0,(LPARAM)"NEC32");
			SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_ADDSTRING,0,(LPARAM)"NEC40");
			
			SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_SETCURSEL,settings.getDeviceNumber(),0);
			SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_SETCURSEL,settings.getRcType(),0);

			ShowWindow(hwnd, SW_SHOW);

			return TRUE;
		}

		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK: {
					settings.setDeviceNumber(SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_GETCURSEL,0,0));
					settings.setRcType(SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_GETCURSEL,0,0));
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

	hDialog = CreateDialog((HINSTANCE)(&__ImageBase),MAKEINTRESOURCE(IDD_DIALOG_CFG),NULL,dialogProc);

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

WL_API int decodeIR(struct ir_remote *remotes, char *out) {

	//wait till data is ready

	if(receive)
	{
		receive->waitTillDataIsReady(0);

		last = end;
		gettimeofday(&start,NULL);
		receive->getData(&irCode);
		gettimeofday(&end,NULL);

		if(decodeCommand(remotes,out)) {
			ResetEvent(dataReadyEvent);
			return 1;
		}

		ResetEvent(dataReadyEvent);
	}

	return 0;
}

WL_API struct hardware* getHardware() {

	initHardwareStruct();
	return &hw;
}
