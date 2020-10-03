#include "ChinavisionAPI.h"
#include <stdio.h>
#include "../Common/LIRCDefines.h"
#include <initguid.h>
#include <setupapi.h>
#include <tchar.h>

using namespace std::chrono;

static const GUID GUID_CLASS_STREAMZAP = { 0x72679574, 0x1865, 0x499d, { 0xb1, 0x82, 0x4b, 0x09, 0x9d, 0x6d, 0x13, 0x91 } };

DWORD WINAPI SZThread(void *recieveClass) {

	((ChinavisionAPI*)recieveClass)->threadProc();
	return 0;
}

ChinavisionAPI::ChinavisionAPI() {

	m_threadHandle		= nullptr;
	m_usbHandle			= nullptr;
	m_deviceHandle		= nullptr;
	m_exitEvent			= nullptr;
	m_dataReadyEvent	= nullptr;
	m_threadExitEvent	= nullptr;

	QueryPerformanceFrequency(&m_frequency);
	QueryPerformanceCounter(&m_lastTime);
}

BOOL ChinavisionAPI::init(HANDLE threadExit) {

	//==========================================
	USB_INTERFACE_DESCRIPTOR	ifaceDescriptor;
	WINUSB_PIPE_INFORMATION		pipeInfo;
	//==========================================
		
	m_usbHandle			= nullptr;
	m_deviceHandle		= nullptr;
	m_inPipeId			= 0;
	m_irCode			= 0;
	m_dataReadyEvent	= CreateEvent(nullptr,TRUE,FALSE,nullptr);
	m_threadExitEvent	= threadExit;

	findDevice();

	if(!_tcslen(m_deviceName)) {
		return FALSE;			// failed to find device name
	}

	m_deviceHandle = CreateFile(m_deviceName,GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);

	if(m_deviceHandle==nullptr) {
		return FALSE;
	}

	if(!WinUsb_Initialize(m_deviceHandle, &m_usbHandle)) {
		cleanUp();
		return FALSE;
	}

	if(!WinUsb_QueryInterfaceSettings(m_usbHandle, 0, &ifaceDescriptor)) {
		cleanUp();
		return FALSE;
	}

	for (int i=0; i<ifaceDescriptor.bNumEndpoints; i++) {
		
		if(!WinUsb_QueryPipe(m_usbHandle, 0, (UCHAR) i, &pipeInfo)) {
			cleanUp();
			return FALSE;
		}

		if (USB_ENDPOINT_DIRECTION_IN(pipeInfo.PipeId)) {
			m_inPipeId = pipeInfo.PipeId;
		}
	}

	if (m_inPipeId==0) {
		cleanUp();
		return FALSE;
	}

	m_threadHandle = CreateThread(nullptr,0,SZThread,(void *)this,0,nullptr);

	if(m_threadHandle) return TRUE;

	return FALSE;
}

void ChinavisionAPI::deinit() {

	killThread();

	if(m_dataReadyEvent) {
		CloseHandle(m_dataReadyEvent);
		m_dataReadyEvent = nullptr;
	}

	if(m_exitEvent) {
		CloseHandle(m_exitEvent);
		m_exitEvent = nullptr;
	}
}

void ChinavisionAPI::threadProc() {

	//========================
	OVERLAPPED	overlappedRead;
	HANDLE		events[2];
	DWORD		result;
	UCHAR		buffer[4];
	ULONG		length;
	//========================

	memset(&overlappedRead,0,sizeof(OVERLAPPED));

	overlappedRead.hEvent = CreateEvent(nullptr,FALSE,FALSE,nullptr);
	m_exitEvent = CreateEvent(nullptr,TRUE,FALSE,nullptr);

	events[0] = overlappedRead.hEvent;
	events[1] = m_exitEvent;

	while(1) {

		memset(buffer,0,sizeof(buffer));	//sometiemes only reads 3 bytes

		WinUsb_ReadPipe(m_usbHandle, m_inPipeId, buffer, sizeof(buffer), &length, &overlappedRead);

		result = WaitForMultipleObjects(2,events,FALSE,INFINITE);

		if(result==WAIT_OBJECT_0) {

			//=============
			DWORD tempCode;
			//=============

			tempCode = *(DWORD*)buffer;
			printf("irCode %x\n",tempCode>>8);

			printf("length %i\n",length);

			for(DWORD i=0; i<length; i++) {
				printf("buffer[%i]",(int)buffer[i]);
			}
			printf("\n");

			tempCode = tempCode >> 8;

			if(tempCode) {
				m_irCode = *(DWORD*)buffer;
				SetEvent(m_dataReadyEvent);
			}
		}
		else {
			break;
		}
	}

	if(m_exitEvent) {
		CloseHandle(m_exitEvent);
		m_exitEvent = nullptr;
	}

	if(overlappedRead.hEvent) {
		CloseHandle(overlappedRead.hEvent);
		overlappedRead.hEvent = nullptr;
	}

	cleanUp();
}

void ChinavisionAPI::waitTillDataIsReady(microseconds maxUSecs) {

	HANDLE events[2]={m_dataReadyEvent,m_threadExitEvent};
	int evt;
	if(m_threadExitEvent==nullptr) evt=1;
	else evt=2;

	if(m_irCode==0)
	{
		ResetEvent(m_dataReadyEvent);
		using namespace std::chrono;
		DWORD const dwTimeout = maxUSecs > 0us
			? duration_cast<milliseconds>(maxUSecs + 500us).count()
			: INFINITE;
		DWORD const res = ::WaitForMultipleObjects(2, events, false, dwTimeout);
		if(res==(WAIT_OBJECT_0+1))
		{
			ExitThread(0);
			return;
		}
	}
}

void ChinavisionAPI::killThread() {

	if(m_exitEvent) {
		SetEvent(m_exitEvent);
	}

	if(m_threadHandle!=nullptr) {

		//===========
		DWORD result;
		//===========

		result = 0;

		if(GetExitCodeThread(m_threadHandle,&result)==0) 
		{
			CloseHandle(m_threadHandle);
			m_threadHandle = nullptr;
			return;
		}

		if(result==STILL_ACTIVE)
		{
			WaitForSingleObject(m_threadHandle,INFINITE);
		}

		CloseHandle(m_threadHandle);
		m_threadHandle = nullptr;
	}
}

int ChinavisionAPI::decodeCommand(char *out) {

	//=====================
	int		tempCode;
	int		reportID;
	char	buttonName[32];
	int		repeats = 0;
	//=====================

	buttonName[0] = '\0';

	tempCode = m_irCode >> 8;		// remove report ID value
	reportID = m_irCode & 0xFF;

	//
	// pointer
	//
	if(reportID==1) {

		switch(tempCode)
		{
		case 0x3D04:
			strcpy_s(buttonName,_countof(buttonName),"Close");
			break;
		case 0x4B00:
			strcpy_s(buttonName,_countof(buttonName),"Page+");
			break;
		case 0x4E00:
			strcpy_s(buttonName,_countof(buttonName),"Page-");
			break;
		case 0x808:
			strcpy_s(buttonName,_countof(buttonName),"MyPC");
			break;
		case 0x2A00:
			strcpy_s(buttonName,_countof(buttonName),"Backspace");
			break;
		case 0x3:
			strcpy_s(buttonName,_countof(buttonName),"Numlock");
			break;
		case 0x708:
			strcpy_s(buttonName,_countof(buttonName),"Desktop");
			break;
		}

	}
	//
	// mouse
	//
	else if(reportID==2) {

		//===========
		UCHAR mouseX;
		UCHAR mouseY;
		//===========

		mouseX = (tempCode >> 8) & 0xFF;
		mouseY = (tempCode >> 16);

		if(mouseX && !mouseY) {
			
			if(mouseX>=230) {
				strcpy_s(buttonName,_countof(buttonName),"Mouse-Left");
			}
			else {
				strcpy_s(buttonName,_countof(buttonName),"Mouse-Right");
			}
		}
		else if(!mouseX && mouseY) {

			if(mouseY>=230) {
				strcpy_s(buttonName,_countof(buttonName),"Mouse-Up");
			}
			else {
				strcpy_s(buttonName,_countof(buttonName),"Mouse-Down");
			}
		}
		else if(mouseX && mouseY) {

			if(mouseX>=230 && mouseY>=230) {
				strcpy_s(buttonName,_countof(buttonName),"Mouse-Up-Left");
			}
			else if(mouseX>=230 && mouseY<=30) {
				strcpy_s(buttonName,_countof(buttonName),"Mouse-Down-Left");
			}
			else if(mouseX<=30 && mouseY>=230) {
				strcpy_s(buttonName,_countof(buttonName),"Mouse-Up-Right");
			}
			else if(mouseX<=30 && mouseY<=30) {
				strcpy_s(buttonName,_countof(buttonName),"Mouse-Down-Right");
			}
		}
		else {

			//=============
			UCHAR mButtons;
			//=============

			mButtons = tempCode & 0xFF;

			if(mButtons==32) {
				strcpy_s(buttonName,_countof(buttonName),"MouseButton-Left");
			}
			else {
				strcpy_s(buttonName,_countof(buttonName),"MouseButton-Right");
			}
		}

	}
	//
	// reserved
	//
	else if(reportID==3) {

		switch(tempCode)
		{
		case 0x18A:
			strcpy_s(buttonName,_countof(buttonName),"E-mail");
			break;
		case 0x223:
			strcpy_s(buttonName,_countof(buttonName),"WWW");
			break;
		case 0xB6:
			strcpy_s(buttonName,_countof(buttonName),"Rewind");
			break;
		case 0xCD:
			strcpy_s(buttonName,_countof(buttonName),"Play");
			break;
		case 0xB5:
			strcpy_s(buttonName,_countof(buttonName),"FastForard");
			break;
		case 0xCB:
			strcpy_s(buttonName,_countof(buttonName),"SkipBackwards");
			break;
		case 0xB7:
			strcpy_s(buttonName,_countof(buttonName),"Stop");
			break;
		case 0xCA:
			strcpy_s(buttonName,_countof(buttonName),"SkipForwards");
			break;
		case 0xE9:
			strcpy_s(buttonName,_countof(buttonName),"Vol+");
			break;
		case 0xEA:
			strcpy_s(buttonName,_countof(buttonName),"Vol-");
			break;
		case 0x230:
			strcpy_s(buttonName,_countof(buttonName),"FullScreen");
			break;
		case 0xE2:
			strcpy_s(buttonName,_countof(buttonName),"Mute");
			break;
		}

	}
	//
	// joystick
	//
	else if(reportID==4) {

		switch(tempCode)
		{
		case 0x1E05:
			strcpy_s(buttonName,_countof(buttonName),"A");
			break;
		case 0x1F05:
			strcpy_s(buttonName,_countof(buttonName),"B");
			break;
		case 0x2005:
			strcpy_s(buttonName,_countof(buttonName),"C");
			break;
		case 0x2105:
			strcpy_s(buttonName,_countof(buttonName),"D");
			break;
		}
	}
	//
	// gamepad
	//
	else if(reportID==5) {

		switch(tempCode)
		{
		case 0x2B00:
			strcpy_s(buttonName,_countof(buttonName),"Tab");
			break;
		case 0x40000:
			strcpy_s(buttonName,_countof(buttonName),"Up");
			break;
		case 0x8:
			strcpy_s(buttonName,_countof(buttonName),"Windows");
			break;
		case 0x5000:
			strcpy_s(buttonName,_countof(buttonName),"Left");
			break;
		case 0x2800:
			strcpy_s(buttonName,_countof(buttonName),"Enter");
			break;
		case 0x4F00:
			strcpy_s(buttonName,_countof(buttonName),"Right");
			break;
		case 0x1201:
			strcpy_s(buttonName,_countof(buttonName),"Open");
			break;
		case 0x5100:
			strcpy_s(buttonName,_countof(buttonName),"Down");
			break;
		case 0x2900:
			strcpy_s(buttonName,_countof(buttonName),"Esc");
			break;
		case 0x2C0000:
			strcpy_s(buttonName,_countof(buttonName),"Cascade");
			break;

		}
	}
	//
	// keyboard
	//
	else if(reportID==6) {

		switch(tempCode)
		{
		case 0x81:
			strcpy_s(buttonName,_countof(buttonName),"Power");
			break;
		}
	}


	_snprintf_s(out,PACKET_SIZE+1,PACKET_SIZE+1,"%016llx %02x %s %s\n",__int64(0),repeats,buttonName,"PC_Remote");

	m_irCode = 0;

	return 1;
}

void ChinavisionAPI::findDevice() {

	//=================================
	HDEVINFO		hardwareDeviceInfo;
	SP_DEVINFO_DATA deviceInfoData;
	//=================================

	m_deviceName[0] = '\0';

	hardwareDeviceInfo = SetupDiGetClassDevs(&GUID_CLASS_STREAMZAP,nullptr,nullptr,(DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)); 

	printf("got here\n");

	if (hardwareDeviceInfo == INVALID_HANDLE_VALUE) {
		printf("Could not find any devices.\n");
        return;
	}

	for(int i=0; ; i++) {

		memset(&deviceInfoData, 0, sizeof(deviceInfoData));
        deviceInfoData.cbSize = sizeof(deviceInfoData);

		if(SetupDiEnumDeviceInfo(hardwareDeviceInfo,i,&deviceInfoData)) {

			for(int j=0; ; j++) {

				//===========================================
				SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
				//===========================================

				memset(&deviceInterfaceData, 0, sizeof(deviceInterfaceData));
				deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

				if(SetupDiEnumDeviceInterfaces(hardwareDeviceInfo,&deviceInfoData,&GUID_CLASS_STREAMZAP,j,&deviceInterfaceData)) {

					//
					// get the size needed for interface details
					//

					//=======================================================
					DWORD requiredSize;
					PSP_DEVICE_INTERFACE_DETAIL_DATA functionClassDeviceData;
					//=======================================================

					SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr);

					functionClassDeviceData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc (requiredSize);
					memset(functionClassDeviceData, 0, requiredSize);
					functionClassDeviceData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

					if (! SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo,&deviceInterfaceData,functionClassDeviceData,requiredSize,nullptr,nullptr)) {
						free(functionClassDeviceData);
						printf("failed here ..");
						break;
					}

					_tprintf(_T("device name: %s\n"),functionClassDeviceData->DevicePath);

					_tcscpy_s(m_deviceName,_countof(m_deviceName),functionClassDeviceData->DevicePath);

					free(functionClassDeviceData);

					return;
				}
				else {

					printf("Could not find any devices. here %i\n",j);
					break;
				}
			}
		}
		else {

			printf("Could not find any devices.. here.");

			if (ERROR_NO_MORE_ITEMS == GetLastError()) {
				break;
			}
                
			break;	//exit loop here anyway
		}
	}

	 SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
}

void ChinavisionAPI::cleanUp() {

	if(m_deviceHandle) {
		CloseHandle(m_deviceHandle);
		m_deviceHandle = nullptr;
	}

	if (m_usbHandle) {
		WinUsb_Free(m_usbHandle);
		m_usbHandle = nullptr;
	}
}

