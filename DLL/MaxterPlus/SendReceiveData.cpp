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

#include <windows.h>
#include "SendReceiveData.h"
#include "Globals.h"
#include <tchar.h>
#include <stdio.h>
#include "../Common/Win32Helpers.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

DWORD WINAPI MaxterPlus(void *recieveClass) {

	((SendReceiveData*)recieveClass)->threadProc();
	return 0;
}

SendReceiveData::SendReceiveData() {

	threadHandle	= nullptr;
	exitEvent		= nullptr;
}

bool SendReceiveData::init() {

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

			threadHandle = CreateThread(nullptr,0,MaxterPlus,(void *)this,0,nullptr);

			if(threadHandle) {
				return true;
			}
		}
	}

	return false;
}

void SendReceiveData::deinit() {

	KillThread(exitEvent,threadHandle);
}

void SendReceiveData::threadProc() {

	//========================
	OVERLAPPED	overlappedRead;
	HANDLE		events[2];
	DWORD		result;
	//========================

	memset(&overlappedRead,0,sizeof(OVERLAPPED));

	overlappedRead.hEvent = CreateEvent(nullptr,FALSE,FALSE,nullptr);
	exitEvent = CreateEvent(nullptr,TRUE,FALSE,nullptr);

	events[0] = overlappedRead.hEvent;
	events[1] = exitEvent;

	setFeatures();

	while(1) {

		//==============
		UCHAR buffer[5];
		DWORD bytesRead;
		//==============

		if(!ReadFile(device.HidDevice,buffer,5,&bytesRead,&overlappedRead)) {
			//break;
		}

		result = WaitForMultipleObjects(2,events,FALSE,INFINITE);

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

bool SendReceiveData::waitTillDataIsReady(std::chrono::microseconds maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==nullptr) evt=1;
	else evt=2;

	if(irCode==0)
	{
		ResetEvent(dataReadyEvent);
		using namespace std::chrono;
		DWORD const dwTimeout = maxUSecs > 0us
			? duration_cast<milliseconds>(maxUSecs + 500us).count()
			: INFINITE;
		DWORD const res = ::WaitForMultipleObjects(2, events, false, dwTimeout);
		if(res==(WAIT_OBJECT_0+1))
		{
			return false;
		}
	}

	return true;
}

BOOL SendReceiveData::setFeatures() {

	device.FeatureReportBuffer[0] = 6;
	device.FeatureReportBuffer[1] = (CHAR)0xFF;	//to shut the compiler up
	device.FeatureReportBuffer[2] = (CHAR)settings.getSettings();
	device.FeatureReportBuffer[3] = 0;
	device.FeatureReportBuffer[4] = 0;

	return HidD_SetFeature (device.HidDevice, device.FeatureReportBuffer, device.Caps.FeatureReportByteLength);

}

void SendReceiveData::restoreFeatures() {

	device.FeatureReportBuffer[0] = 6;
	device.FeatureReportBuffer[1] = (CHAR)0xFF;	//to shut the compiler up
	device.FeatureReportBuffer[2] = 0x40;
	device.FeatureReportBuffer[3] = 0;
	device.FeatureReportBuffer[4] = 0;

	HidD_SetFeature (device.HidDevice, device.FeatureReportBuffer, device.Caps.FeatureReportByteLength);
}

int SendReceiveData::decodeCommand(char *out, size_t out_size) {

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

	_snprintf_s(out, out_size,PACKET_SIZE+1,"%016llx %02x %s %s\n",__int64(0),repeats,buttonName,"MaxterPlus");

	return 1;

}


