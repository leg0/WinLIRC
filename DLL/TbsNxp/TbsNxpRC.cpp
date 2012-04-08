#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#include "TbsNxpRC.h"
#include "Globals.h"
#include "resource.h"
#include "Receive.h"
#include "Settings.h"
#include "hardware.h"
#include "decode.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

IG_API int init(HANDLE exitEvent) {
	initHardwareStruct();

	threadExitEvent = exitEvent;
	dataReadyEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);

	receive = new Receive();
	return receive->init(settings.getDeviceNumber(),(tmIrDecoderId)settings.getRcType());

}

IG_API void deinit() {

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

IG_API int hasGui() {

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
			ICreateDevEnum*	pSysDevEnum = NULL;	
			HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
			if (hr == S_OK)
			{
				// create a class enumerator for the desired category defined by classGuid.
				IEnumMoniker* pEnumCat = NULL;	//moniker enumerator for filter categories
				hr = pSysDevEnum->CreateClassEnumerator(CLSID_DeviceControlCategory, &pEnumCat, 0);
				pSysDevEnum->Release();
				if (hr == S_OK)
				{

					// reset the enumeration
					pEnumCat->Reset();

					// now iterate through enumeration
					ULONG cFetched = 0;
					IMoniker* pMoniker = NULL;

					// get a pointer to each moniker
					while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
					{
						//get a pointer to property bag (which has filter)
						IPropertyBag* pPropBag = NULL;	
						hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
						if (hr != S_OK)
						{
							pEnumCat->Release();
							pMoniker->Release();
							break;
						}

						TCHAR szFriendlyName[MAX_PATH];
						VARIANT varName;
						// retrieve the friendly name of the filter
						VariantInit(&varName);
						hr = pPropBag->Read(L"FriendlyName", &varName, 0);
						WideCharToMultiByte(CP_ACP, 0, varName.bstrVal, -1, szFriendlyName, sizeof(szFriendlyName), 0, 0);
						VariantClear(&varName);
						if ( strstr(szFriendlyName,"TBS") || strstr(szFriendlyName,"TT-budget") )
						{
							PTCHAR t=strchr(strchr(szFriendlyName,' ')+1,' ');
							if (t) t[0]=0;
							SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_ADDSTRING,0,(LPARAM)szFriendlyName);
							numberOfDevices++;
						}
						pMoniker->Release();
						pPropBag->Release();
					}
				}

				pEnumCat->Release();
			}
			
			if (settings.getDeviceNumber()>=numberOfDevices)
				settings.setDeviceNumber(0);

			SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_SETITEMDATA,
							   SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_ADDSTRING,0,(LPARAM)"RC5"),
							   IR_DECODER_ID_RC5);
			SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_SETITEMDATA,
							   SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_ADDSTRING,0,(LPARAM)"RC6"),
							   IR_DECODER_ID_RC6);
			SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_SETITEMDATA,
							   SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_ADDSTRING,0,(LPARAM)"NEC32"),
							   IR_DECODER_ID_NEC_32);
			SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_SETITEMDATA,
							   SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_ADDSTRING,0,(LPARAM)"NEC40"),
							   IR_DECODER_ID_NEC_40);
			
			SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_SETCURSEL,settings.getDeviceNumber(),0);
			SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_SETCURSEL,settings.getRcType(),0);

			ShowWindow( GetDlgItem(hwnd,IDC_COMBO_DEVID), numberOfDevices ? SW_SHOW : SW_HIDE);
			ShowWindow( GetDlgItem(hwnd,IDC_COMBO_RCTYPE), numberOfDevices ? SW_SHOW : SW_HIDE);

			ShowWindow(hwnd, SW_SHOW);

			return TRUE;
		}

		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK: {
					settings.setDeviceNumber(SendDlgItemMessage(hwnd,IDC_COMBO_DEVID,CB_GETCURSEL,0,0));
					settings.setRcType(SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_GETITEMDATA,
									   SendDlgItemMessage(hwnd,IDC_COMBO_RCTYPE,CB_GETCURSEL,0,0),0));
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

IG_API void	loadSetupGui() {

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

IG_API int sendIR(struct ir_remote *remote, struct ir_ncode *code, int repeats) {

	return 0;
}

IG_API int decodeIR(struct ir_remote *remotes, char *out) {

	//wait till data is ready

	if(receive)
	{
		receive->waitTillDataIsReady(0);

		last = end;
		gettimeofday(&start,NULL);
		receive->getData(&irCode);
		gettimeofday(&end,NULL);

		if(decodeCommand(remotes,out)) {
			return 1;
		}
	}

	return 0;
}

IG_API struct hardware* getHardware() {

	initHardwareStruct();
	return &hw;
}
