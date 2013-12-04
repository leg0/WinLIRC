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

/*
So this device is a little weird. Half the keys act like a normal keyboard.
The other half I have to read the raw hid data. So I actually have 2 threads
each reading keys using 2 different methods. RawInput and raw hid data. It works though.
There's no thread checking, but oh well. What's the worst that could happen ? :D
*/

#include <windows.h>
#include "SendReceiveData.h"
#include "Globals.h"
#include <stdio.h>
#include <tchar.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

SendReceiveData *sendRec = NULL;	// this wont work if we have multiple instances, but we don't so no worries

DWORD WINAPI FireFly(void *recieveClass) {

	((SendReceiveData*)recieveClass)->threadProc();
	return 0;
}

SendReceiveData::SendReceiveData() {

	threadHandle	= NULL;
	exitEvent		= NULL;
	windowHandle	= NULL;
	rawInputDevice	= NULL;

	sendRec			= this;
}


BOOL SendReceiveData::init() {

	//==========================
	PHID_DEVICE	devices;
	ULONG		numberOfDevices;
	BOOL		deviceFound;
	TCHAR		deviceName[1024];
	//==========================

	deviceFound		= 0;
	irCode			= 0;
	irLastCode		= 0;
	repeats			= 0;
	rawInputDevice	= NULL;

	if(FindKnownHidDevices(&devices,&numberOfDevices)) {

		for(UINT i=0; i<numberOfDevices; i++) {

			if(devices[i].Attributes.ProductID==57351) {
				if(devices[i].Attributes.VendorID==4659) {
					if(devices[i].Caps.UsagePage==12) {
						if(devices[i].Caps.Usage==1) {
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

			threadHandle = CreateThread(NULL,0,FireFly,(void *)this,0,NULL);

			if(threadHandle) {

				createWindow();

				if(registerRawDevice()) {
					return TRUE;
				}
			}
		}
	}

	return false;
}

void SendReceiveData::deinit() {

	killThread();
	destroyWindow();
}


void SendReceiveData::threadProc() {

	//========================
	OVERLAPPED	overlappedRead;
	HANDLE		events[2];
	DWORD		result;
	//========================

	memset(&overlappedRead,0,sizeof(OVERLAPPED));

	overlappedRead.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	exitEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	events[0] = overlappedRead.hEvent;
	events[1] = exitEvent;


	while(1) {

		//==============
		UCHAR buffer[2];
		DWORD bytesRead;
		//==============

		if(!ReadFile(device.HidDevice,buffer,2,&bytesRead,&overlappedRead)) {
			//break;
		}

		result = WaitForMultipleObjects(2,events,FALSE,INFINITE);

		if(result==(WAIT_OBJECT_0)) 
		{
			if(buffer[1]) {
				irCode = buffer[1];
				repeats = 0;			// there are never any repeats for these keys
				SetEvent(dataReadyEvent);
			}
			
		}

		if(result==(WAIT_OBJECT_0+1))
		{
			break;
		}
	}

	CloseHidDevice(&device);

	if(exitEvent) {
		CloseHandle(exitEvent);
		exitEvent = NULL;
	}

	if(overlappedRead.hEvent) {
		CloseHandle(overlappedRead.hEvent);
		overlappedRead.hEvent = NULL;
	}

}

void SendReceiveData::killThread() {

	//
	// need to kill thread here
	//
	if(exitEvent) {
		SetEvent(exitEvent);
	}

	if(threadHandle!=NULL) {

		//===========
		DWORD result;
		//===========

		result = 0;

		if(GetExitCodeThread(threadHandle,&result)==0) 
		{
			CloseHandle(threadHandle);
			threadHandle = NULL;
			return;
		}

		if(result==STILL_ACTIVE)
		{
			WaitForSingleObject(threadHandle,INFINITE);
		}

		CloseHandle(threadHandle);
		threadHandle = NULL;
	}
}

bool SendReceiveData::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==NULL) evt=1;
	else evt=2;

	if(irCode==0)
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

int SendReceiveData::decodeCommand(char *out) {

	//==================
	UINT outCode;
	char buttonName[32];
	//==================

	outCode = irCode;

	switch(outCode) {
		
		case 26:	strcpy_s(buttonName,_countof(buttonName),"CLOSE");	break;
		case 17:	strcpy_s(buttonName,_countof(buttonName),"MUTE");	break;
		case 13:	strcpy_s(buttonName,_countof(buttonName),"VOL+");	break;
		case 14:	strcpy_s(buttonName,_countof(buttonName),"VOL-");	break;
		case 2:		strcpy_s(buttonName,_countof(buttonName),"CH+");	break;
		case 3:		strcpy_s(buttonName,_countof(buttonName),"CH-");	break;
		case 1:		strcpy_s(buttonName,_countof(buttonName),"LAST");	break;
		case 24:	strcpy_s(buttonName,_countof(buttonName),"EXIT");	break;
		case 27:	strcpy_s(buttonName,_countof(buttonName),"OPTION");	break;
		case 16:	strcpy_s(buttonName,_countof(buttonName),"FIREFLY");break;
		case 15:	strcpy_s(buttonName,_countof(buttonName),"MENU");	break;
		case 11:	strcpy_s(buttonName,_countof(buttonName),"REC");	break;
		case 20:	strcpy_s(buttonName,_countof(buttonName),"GUIDE");	break;
		case 9:		strcpy_s(buttonName,_countof(buttonName),"STOP");	break;
		case 8:		strcpy_s(buttonName,_countof(buttonName),"PREV");	break;
		case 4:		strcpy_s(buttonName,_countof(buttonName),"PLAY");	break;
		case 7:		strcpy_s(buttonName,_countof(buttonName),"NEXT");	break;
		case 6:		strcpy_s(buttonName,_countof(buttonName),"REW");	break;
		case 10:	strcpy_s(buttonName,_countof(buttonName),"PAUSE");	break;
		case 5:		strcpy_s(buttonName,_countof(buttonName),"FWD");	break;

		//
		// added 100 to 'keyboard' keys to stop collision with above data
		//

		case 148:	strcpy_s(buttonName,_countof(buttonName),"0");		break;
		case 149:	strcpy_s(buttonName,_countof(buttonName),"1");		break;
		case 150:	strcpy_s(buttonName,_countof(buttonName),"2");		break;
		case 151:	strcpy_s(buttonName,_countof(buttonName),"3");		break;
		case 152:	strcpy_s(buttonName,_countof(buttonName),"4");		break;
		case 153:	strcpy_s(buttonName,_countof(buttonName),"5");		break;
		case 154:	strcpy_s(buttonName,_countof(buttonName),"6");		break;
		case 155:	strcpy_s(buttonName,_countof(buttonName),"7");		break;
		case 156:	strcpy_s(buttonName,_countof(buttonName),"8");		break;
		case 157:	strcpy_s(buttonName,_countof(buttonName),"9");		break;

		case 137:	strcpy_s(buttonName,_countof(buttonName),"LEFT");	break;
		case 139:	strcpy_s(buttonName,_countof(buttonName),"RIGHT");	break;
		case 138:	strcpy_s(buttonName,_countof(buttonName),"UP");		break;
		case 140:	strcpy_s(buttonName,_countof(buttonName),"DOWN");	break;
		case 113:	strcpy_s(buttonName,_countof(buttonName),"OK");		break;

		default: {
			irCode	= 0;
			return 0;
		}

	}

	irLastCode	= outCode;
	irCode		= 0;

	_snprintf_s(out,PACKET_SIZE+1,PACKET_SIZE+1,"%016llx %02x %s %s\n",__int64(0),repeats,buttonName,"FireFlyMini");

	return 1;
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_INPUT:
		{
			//====================
			UINT		size;
			INT			result;
			UCHAR		*data;
			RAWINPUT	*rawInput;
			//====================

			size	= 0;
			data	= NULL;

			result = GetRawInputData((HRAWINPUT)lParam,RID_INPUT,NULL,&size,sizeof(RAWINPUTHEADER));

			if(result<0) return 0;	// failed somehow

			data = new UCHAR[size];

			result = GetRawInputData((HRAWINPUT)lParam,RID_INPUT,data,&size,sizeof(RAWINPUTHEADER));

			if(result<=0) {
				delete []data;
				return 0;		//failed somehow
			}

			rawInput = (RAWINPUT*)data;


			//
			// do we have a handle to the right device
			//
			if(rawInput->header.hDevice==sendRec->rawInputDevice) {


				if(!(rawInput->data.keyboard.Flags & RI_KEY_BREAK)) {

					if(sendRec->irLastCode == (UCHAR)rawInput->data.keyboard.VKey + 100) {
						sendRec->repeats++;
					}
					else {
						sendRec->repeats=0;
					}

					sendRec->irCode = (UCHAR)rawInput->data.keyboard.VKey + 100;

					SetEvent(dataReadyEvent);
				}
				else {
					sendRec->irLastCode = 0;
					sendRec->repeats	= 0;
				}

			}

			delete []data;

			return 0;
		}


	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void SendReceiveData::createWindow() {

	//===============
	WNDCLASSEX	wcex;
	//===============

	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= (HINSTANCE)(&__ImageBase);
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= _T("FireFlyReceiveWindow");
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	windowHandle = CreateWindowEx(0,_T("FireFlyReceiveWindow"),_T("FireFly"),WS_POPUP,0,0,1,1,NULL,NULL,(HINSTANCE)(&__ImageBase),NULL);
}

void SendReceiveData::destroyWindow() {

	DestroyWindow	(windowHandle);
	UnregisterClass	("FireFlyReceiveWindow",(HINSTANCE)(&__ImageBase));

	windowHandle = NULL;
}

BOOL SendReceiveData::registerRawDevice() {

	//=============================
	UINT numberOfDevices;
	RAWINPUTDEVICELIST *deviceList;
	int result;
	bool deviceFound;
	//=============================

	numberOfDevices = 0;
	deviceList		= NULL;
	deviceFound		= false;

	result = GetRawInputDeviceList(NULL,&numberOfDevices,sizeof(RAWINPUTDEVICELIST));

	if(result<0) return false;	//error

	deviceList = new RAWINPUTDEVICELIST[numberOfDevices];

	result = GetRawInputDeviceList(deviceList,&numberOfDevices,sizeof(RAWINPUTDEVICELIST));

	if(result<=0) {
		return false;	//no devices or error
	}

	for(UINT i=0; i<numberOfDevices; i++) {

		//===============
		char	*deviceName;
		UINT	size;
		INT		result;
		//===============

		size		= 0;
		deviceName	= NULL;
		result		= 0;

		result = GetRawInputDeviceInfo(deviceList[i].hDevice,RIDI_DEVICENAME,NULL,&size);

		if(result<0) continue;

		deviceName = new char[size];

		GetRawInputDeviceInfo(deviceList[i].hDevice,RIDI_DEVICENAME,deviceName,&size);

		toUpperCase(deviceName);

		if(deviceList[i].dwType==RIM_TYPEKEYBOARD && strstr(deviceName,"VID_1233&PID_E007")) {

			rawInputDevice	= deviceList[i].hDevice;
			deviceFound		= true;

			delete [] deviceName;

			break;
		}

		delete [] deviceName;
	}


	//
	// clean up
	//
	delete [] deviceList;

	if(!deviceFound) return false;

	//
	// register device for raw input
	//

	{
		//============================
		RAWINPUTDEVICE rawInputDevice;
		//============================

		rawInputDevice.dwFlags		= RIDEV_INPUTSINK;
		rawInputDevice.hwndTarget	= windowHandle;
		rawInputDevice.usUsage		= 0x06;
		rawInputDevice.usUsagePage	= 0x01;

		return RegisterRawInputDevices(&rawInputDevice,1,sizeof(RAWINPUTDEVICE));

	}
}

void SendReceiveData::toUpperCase(char *string) {

	while(*string) { 
		*string = toupper(*string); 
		string++; 
	}
}


