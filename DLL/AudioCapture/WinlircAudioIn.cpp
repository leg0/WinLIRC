#include <Windows.h>
#include "LIRCDefines.h"
#include "WinlircAudioIn.h"
#include "resource.h"
#include <tchar.h>
#include "Globals.h"
#include <stdio.h>
#include "AudioFormats.h"
#include <atlstr.h>
#include "Decode.h"
#include "hardware.h"

//For the GUI
//==============================
int			formatArray[50];
int			indexNumber = 0;
Settings	*guiSettings = NULL;
//==============================

SI_API int init(HANDLE exitEvent) {

	//=====================
	TCHAR	deviceName[32];
	int		deviceID;
	int		audioFormat;
	int		frequency;
	BOOL	stereo;
	bool	left;
	//=====================

	settings		= new Settings();
	recordAudio		= new RecordAudio();
	dataReadyEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);
	threadExitEvent	= exitEvent;

	init_rec_buffer();

	settings->loadSettings();
	settings->getAudioDeviceName(deviceName);

	audioFormat = settings->getAudioFormat();
	left		= settings->getChannel();

	AudioFormats::getFormatDetails(audioFormat,&stereo,&frequency);

	deviceID = AudioFormats::getAudioIndex(deviceName);

	if(deviceID==-1) {
		return 0;
	}
	else {
		recordAudio->startRecording(deviceID,frequency,stereo+1,left);
		return 1; //success
	}
}

SI_API void deinit() {

	if(recordAudio) {
		recordAudio->stopRecording();
		delete recordAudio;
		recordAudio = NULL;
	}

	if(settings) {
		delete settings;
		settings = NULL;
	}

	if(dataReadyEvent) {
		CloseHandle(dataReadyEvent);
		dataReadyEvent = NULL;
	}

	threadExitEvent = NULL;	//this one is created outside the DLL
}

SI_API int hasGui() {

	return TRUE;
}

void addAudioDeviceList(HWND hwnd, int item) {

	//==============================
	UINT		numberOfDevices;
	TCHAR		audioDeviceName[32];
	WAVEINCAPS	caps;
	BOOL		foundDevice;
	UINT		foundIndex;
	//==============================

	numberOfDevices	= waveInGetNumDevs();
	foundDevice		= FALSE;
	foundIndex		= FALSE;

	if(!numberOfDevices) return;			//no audio devices

	guiSettings->getAudioDeviceName(audioDeviceName);

	for(UINT i=0; i<numberOfDevices; i++) {

		//==================
		CString tempString;
		LPCTSTR tempString2;
		//==================

		waveInGetDevCaps(i,&caps,sizeof(caps));

		tempString = caps.szPname;
		tempString.TrimRight();

		tempString2 = tempString;

		SendDlgItemMessage(hwnd,item,CB_ADDSTRING,0,(LPARAM)tempString2);

		_tprintf(_T("%s z\n"),caps.szPname);
		_tprintf(_T("%s z\n"),audioDeviceName);

		if(! _tcscmp(tempString2,audioDeviceName)) {
			foundDevice		= TRUE;
			foundIndex		= i;
		}
	}

	//
	// if we haven't any device listed yet add the first one from the list
	//
	if(! _tcscmp(_T(""),audioDeviceName) ) {

		SendDlgItemMessage(hwnd,item,CB_SETCURSEL,0,0);	//select first item
		waveInGetDevCaps(0,&caps,sizeof(caps));
		guiSettings->setAudioDeviceName(caps.szPname);
	}

	//
	// We found the device so select it
	//
	if(foundDevice) {
		SendDlgItemMessage(hwnd,item,CB_SETCURSEL,foundIndex,0);
	}

}

void addAudioFormats(HWND hwnd, int item) {

	//==================================
	TCHAR		selectedAudioDevice[32];
	int			audioIndex;
	WAVEINCAPS	caps;
	//==================================

	audioIndex = 0;

	SendDlgItemMessage(hwnd,item,CB_RESETCONTENT,0,0);		//reset content

	GetDlgItemText(hwnd,IDC_COMBO1,selectedAudioDevice,32);

	audioIndex = AudioFormats::getAudioIndex(selectedAudioDevice);

	if(MMSYSERR_NOERROR == waveInGetDevCaps(audioIndex,&caps,sizeof(caps)) ) {

		//===============
		int shiftFormat;
		int tempFormat;
		int indexCounter;
		//===============

		shiftFormat		= 1;
		tempFormat		= 0;
		indexCounter	= 0;

		for(int i=0; i<30; i++) {

			tempFormat = caps.dwFormats & shiftFormat;

			//printf("temp format %i\n",tempFormat);

			if(tempFormat) {

				//check we support this one 

				//printf("format supported %i\n",AudioFormats::formatSupported(tempFormat));

				if(AudioFormats::formatSupported(tempFormat)) {

					//===============
					TCHAR string[64];
					//===============

					AudioFormats::getFormatString(tempFormat,string,64);

					//_tprintf(_T("string %s %i\n"),string,tempFormat);

					//add string to combo box thing !
					SendDlgItemMessage(hwnd,item,CB_ADDSTRING,0,(LPARAM)string);
					if(tempFormat==guiSettings->getAudioFormat()) {
						SendDlgItemMessage(hwnd,item,CB_SETCURSEL,indexCounter,0);
					}

					formatArray[indexNumber] = tempFormat;

					indexCounter++;
					indexNumber = indexCounter;
				}
			}

			shiftFormat <<= 1;
		}
	}
}

BOOL CALLBACK dialogProc (HWND hwnd, 
                          UINT message, 
                          WPARAM wParam, 
                          LPARAM lParam) {

	//printf("message %i\n",message);


    switch (message) {

		case WM_INITDIALOG: {

			//==============
			int audioFormat;
			int i;
			int temp;
			int stereo;
			//==============

			SendDlgItemMessage(hwnd,IDC_COMBO3,CB_ADDSTRING,0,(LPARAM)_T("Left Channel"));
			SendDlgItemMessage(hwnd,IDC_COMBO3,CB_ADDSTRING,0,(LPARAM)_T("Right Channel"));
			SendDlgItemMessage(hwnd,IDC_COMBO3,CB_SETCURSEL,0,0);

			addAudioDeviceList(hwnd,IDC_COMBO1);
			addAudioFormats(hwnd,IDC_COMBO2);

			//
			// match audio format to window
			//
			audioFormat = guiSettings->getAudioFormat();

			for(i=0; i< _countof(formatArray); i++) {

				if(audioFormat == formatArray[i]) {
					SendDlgItemMessage(hwnd,IDC_COMBO2,CB_SETCURSEL,i,0);
					break;
				}
			}

			//
			// set left/right channel
			//
			if(guiSettings->getChannel()) {
				SendDlgItemMessage(hwnd,IDC_COMBO3,CB_SETCURSEL,0,0);
			}
			else {
				SendDlgItemMessage(hwnd,IDC_COMBO3,CB_SETCURSEL,1,0);
			}

			//
			// disable / enable CB3
			//
			AudioFormats::getFormatDetails(audioFormat,&stereo,&temp);

			if(stereo) {
				EnableWindow(GetDlgItem(hwnd,IDC_COMBO3),TRUE);
			}
			else {
				EnableWindow(GetDlgItem(hwnd,IDC_COMBO3),FALSE);
				guiSettings->setChannel(true);
			}

			ShowWindow(hwnd, SW_SHOW);

			return TRUE;
		}


		case WM_COMMAND: {

			switch(LOWORD(wParam)) {

				case IDOK: {
					guiSettings->saveSettings();
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

				case IDC_COMBO1: {

					if(HIWORD(wParam)==CBN_SELCHANGE) {
						
						//===============
						TCHAR device[32];
						//===============

						//
						// update selected device
						//

						GetDlgItemText(hwnd,IDC_COMBO1,device,_countof(device));

						guiSettings->setAudioDeviceName(device);

						addAudioFormats(hwnd,IDC_COMBO2);
						SendDlgItemMessage(hwnd,IDC_COMBO2,CB_SETCURSEL,0,0);
					}
				}
				case IDC_COMBO2: {

					if(HIWORD(wParam)==CBN_SELCHANGE) {

						//==============
						int index;
						int audioFormat;
						int temp;
						int stereo;
						//==============
						
						index = SendDlgItemMessage(hwnd,IDC_COMBO2,CB_GETCURSEL,0,0);
						//printf("index value %i\n",index);

						audioFormat = formatArray[index];

						//printf("%i audio format\n",audioFormat);

						//
						// double check it case of cataclysm
						//
						if(AudioFormats::formatSupported(audioFormat)) {
						
							AudioFormats::getFormatDetails(audioFormat,&stereo,&temp);

							if(stereo) {
								EnableWindow(GetDlgItem(hwnd,IDC_COMBO3),TRUE);
							}
							else {
								EnableWindow(GetDlgItem(hwnd,IDC_COMBO3),FALSE);
								guiSettings->setChannel(true);
							}

							guiSettings->setAudioFormat(audioFormat);
						}
					
					}
				}
				case IDC_COMBO3: {

					if(HIWORD(wParam)==CBN_SELCHANGE) {

						//========
						int index;
						//========
						
						index = SendDlgItemMessage(hwnd,IDC_COMBO3,CB_GETCURSEL,0,0);

						if(!index) {
							guiSettings->setChannel(true);
						}
						else {
							guiSettings->setChannel(false);
						}
					}
					
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

SI_API void loadSetupGui() {

	//==============
	HWND	hDialog;
	MSG		msg;
    INT		status;
	//==============

	guiSettings = new Settings();

	hDialog = CreateDialog((HINSTANCE)(&__ImageBase),MAKEINTRESOURCE(IDD_DIALOG1),NULL,dialogProc);

    while ((status = GetMessage (& msg, 0, 0, 0)) != 0) {

        if (status == -1) return;

        if (!IsDialogMessage (hDialog, & msg)) {

            TranslateMessage ( & msg );
            DispatchMessage ( & msg );
        }
    }

	delete guiSettings;
}

SI_API int sendIR(struct ir_remote *remotes, struct ir_ncode *code, int repeats) {

	//
	// return false - since we don't support this function
	//

	return 0;
}

SI_API int decodeIR(struct ir_remote *remotes, char *out) {

	waitTillDataIsReady(0);

	if(decodeCommand(remotes, out)) {
		return 1;
	}
	
	return 0;
}

SI_API struct hardware* getHardware() {

	initHardwareStruct();	//make sure values are setup

	return &hw;
}