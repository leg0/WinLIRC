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
#include "../Common/LIRCDefines.h"
#include "../Common/IRRemote.h"
#include "../Common/Receive.h"
#include "../Common/Hardware.h"
#include "../Common/WLPluginAPI.h"
#include "../Common/Win32Helpers.h"
#include "resource.h"
#include <tchar.h>
#include "Globals.h"
#include "AudioFormats.h"
#include "StringFunctions.h"
#include <Commctrl.h>
#include <cstdio>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
void initHardwareStruct();
extern hardware hw;

//For the GUI
//==============================
int			formatArray[50];
int			indexNumber = 0;
Settings	*guiSettings = nullptr;
//==============================

WL_API int init(HANDLE exitEvent) {

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
	dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);
	threadExitEvent	= exitEvent;

	initHardwareStruct();
	init_rec_buffer();

	settings->loadSettings();
	settings->getAudioDeviceName(deviceName);

	audioFormat = settings->getAudioFormat();
	left		= settings->getChannel();

	AudioFormats::getFormatDetails(audioFormat,&stereo,&frequency);

	deviceID = AudioFormats::getAudioIndex(deviceName);

	if(deviceID<0) {
		return 0;
	}
	else {
		recordAudio->startRecording(deviceID,frequency,stereo+1,left,settings->getPolarity());
		return 1; //success
	}
}

WL_API void deinit() {

	if(recordAudio) {
		recordAudio->stopRecording();
		delete recordAudio;
		recordAudio = nullptr;
	}

	if(settings) {
		delete settings;
		settings = nullptr;
	}

	SAFE_CLOSE_HANDLE(dataReadyEvent);

	threadExitEvent = nullptr;	//this one is created outside the DLL
}

WL_API int hasGui() {

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

		waveInGetDevCaps(i,&caps,sizeof(caps));
		removeTrailingWhiteSpace(caps.szPname);

		SendDlgItemMessage(hwnd,item,CB_ADDSTRING,0,(LPARAM)caps.szPname);

		if(! _tcscmp(caps.szPname,audioDeviceName)) {
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
		removeTrailingWhiteSpace(caps.szPname);
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

			if(tempFormat) {

				//check we support this one 

				if(AudioFormats::formatSupported(tempFormat)) {

					//===============
					TCHAR string[64];
					//===============

					AudioFormats::getFormatString(tempFormat,string,64);

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

INT_PTR CALLBACK dialogProc (HWND hwnd, 
                          UINT message, 
                          WPARAM wParam, 
                          LPARAM lParam) {

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

			SendDlgItemMessage(hwnd,IDC_COMBO4,CB_ADDSTRING,0,(LPARAM)_T("Positive"));
			SendDlgItemMessage(hwnd,IDC_COMBO4,CB_ADDSTRING,0,(LPARAM)_T("Negative"));
			SendDlgItemMessage(hwnd,IDC_COMBO4,CB_ADDSTRING,0,(LPARAM)_T("Auto Detect"));
			SendDlgItemMessage(hwnd,IDC_COMBO4,CB_SETCURSEL,guiSettings->getPolarity(),0);
			
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

			//
			// slider control
			//
			SendDlgItemMessage(hwnd,IDC_SLIDER1,TBM_SETRANGEMIN,FALSE,0);
			SendDlgItemMessage(hwnd,IDC_SLIDER1,TBM_SETRANGEMAX,FALSE,127);
			SendDlgItemMessage(hwnd,IDC_SLIDER1,TBM_SETPOS,TRUE,guiSettings->getNoiseValue());

			//
			// edit box - slider value
			//
			TCHAR editText[4];

			_stprintf_s(editText,_countof(editText),_T("%i"),guiSettings->getNoiseValue());
			SetWindowText(GetDlgItem(hwnd,IDC_EDIT1),editText);

			ShowWindow(hwnd, SW_SHOW);

			return TRUE;
		}

		case WM_HSCROLL: {

			if(lParam==(LPARAM)GetDlgItem(hwnd,IDC_SLIDER1)) {

				//================
				TCHAR editText[4];
				//================

				guiSettings->setNoiseValue(SendDlgItemMessage(hwnd,IDC_SLIDER1,TBM_GETPOS,0,0));
				_stprintf_s(editText,_countof(editText),_T("%i"),guiSettings->getNoiseValue());
				SetWindowText(GetDlgItem(hwnd,IDC_EDIT1),editText);
			}
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
						audioFormat = formatArray[index];

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

						//============
						LRESULT index;
						//============
						
						index = SendDlgItemMessage(hwnd,IDC_COMBO3,CB_GETCURSEL,0,0);

						if(!index) {
							guiSettings->setChannel(true);
						}
						else {
							guiSettings->setChannel(false);
						}
					}
					
				}

				case IDC_COMBO4: {

					if(HIWORD(wParam)==CBN_SELCHANGE) {

						//============
						LRESULT index;
						//============
						
						index = SendDlgItemMessage(hwnd,IDC_COMBO4,CB_GETCURSEL,0,0);

						guiSettings->setPolarity((SP)index);
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

WL_API void loadSetupGui() {

	//==============
	HWND	hDialog;
	MSG		msg;
    INT		status;
	//==============

	guiSettings = new Settings();

	hDialog = CreateDialog((HINSTANCE)(&__ImageBase),MAKEINTRESOURCE(IDD_DIALOG1),nullptr,dialogProc);

    while ((status = GetMessage (& msg, 0, 0, 0)) != 0) {

        if (status == -1) return;

        if (!IsDialogMessage (hDialog, & msg)) {

            TranslateMessage ( & msg );
            DispatchMessage ( & msg );
        }
    }

	delete guiSettings;
}

WL_API int sendIR(struct ir_remote *remotes, struct ir_ncode *code, int repeats) {

	//
	// return false - since we don't support this function
	//

	return 0;
}

WL_API int decodeIR(struct ir_remote *remotes, char *out, size_t out_size) {

	using namespace std::chrono_literals;
	if(!waitTillDataIsReady(0us)) {
		return 0;
	}

	clear_rec_buffer(&hw);

	if(decodeCommand(&hw, remotes, out, out_size)) {
		return 1;
	}
	
	return 0;
}

WL_API struct hardware* getHardware() {

	initHardwareStruct();	//make sure values are setup

	return &hw;
}