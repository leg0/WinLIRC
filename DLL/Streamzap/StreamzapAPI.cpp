#include "StreamzapAPI.h"
#include <stdio.h>
#include "Globals.h"
#include <initguid.h>
#include <setupapi.h>
#include <tchar.h>

DEFINE_GUID(GUID_CLASS_STREAMZAP, 0x990b264a, 0xf1f1, 0x4619, 0x95, 0xe7, 0x0e, 0xc4, 0x1b, 0xff, 0x9f, 0xf8);

#define STREAMZAP_PULSE_MASK 0xf0
#define STREAMZAP_SPACE_MASK 0x0f
#define STREAMZAP_TIMEOUT    0xff
#define STREAMZAP_RESOLUTION 256

DWORD WINAPI SZThread(void *recieveClass) {

	((StreamzapAPI*)recieveClass)->threadProc();
	return 0;
}

StreamzapAPI::StreamzapAPI() {

	threadHandle	= NULL;
	threadExitEvent	= NULL;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&lastTime);
}

BOOL StreamzapAPI::init(HANDLE exit) {

	exitThread		= false;
	nextByteFull	= false;

	bufferStart		= 0;
	bufferEnd		= 0;
	pulse			= PULSE_BIT;
	newSignal		= TRUE;
	deviceName[0]	= '\0';

	findDevice();

	if(!_tcslen(deviceName)) {
		return FALSE;			// failed to find device name
	}

	threadExitEvent = exit;
	dataReadyEvent	= CreateEvent(NULL,TRUE,FALSE,NULL);
	deviceHandle	= CreateFile(deviceName,GENERIC_READ,0,NULL,OPEN_EXISTING,0, NULL); 

	if(deviceHandle==NULL) {
		return FALSE;
	}

	//
	// see if reading works - if not someone else is reading
	//
	{
		//==============
		UCHAR buffer;
		DWORD bytesRead;
		//==============

		if(!ReadFile(deviceHandle,&buffer,1,&bytesRead,NULL)) {
			return FALSE;
		}
	}

	threadHandle = CreateThread(NULL,0,SZThread,(void *)this,0,NULL);

	if(threadHandle) return TRUE;

	return FALSE;
}

void StreamzapAPI::deinit() {

	killThread();

	if(dataReadyEvent) {
		CloseHandle(dataReadyEvent);
		dataReadyEvent = NULL;
	}

	threadExitEvent = NULL;
}

void StreamzapAPI::threadProc() {

	while(1) {

		//==============
		UCHAR buffer;
		DWORD bytesRead;
		//==============

		ReadFile(deviceHandle,&buffer,1,&bytesRead,NULL);
		
		if(bytesRead) {
			//printf("buffer value %i\n",buffer);
			decode(buffer);
		}
		else {
			Sleep(20);
		}

		if(exitThread) break;
	}

	//
	// clean up
	//

	if(deviceHandle) {
		CloseHandle(deviceHandle);
		deviceHandle = NULL;
	}

	//printf("exited thread\n");
}

void StreamzapAPI::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==NULL) evt=1;
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
			ExitThread(0);
		}
	}
}

void StreamzapAPI::killThread() {

	exitThread = true;

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

bool StreamzapAPI::dataReady() {

	if(bufferStart==bufferEnd) return false;
	
	return true;
}

void StreamzapAPI::decode(BYTE data) {

	if(data==STREAMZAP_TIMEOUT) {
		
		newSignal = TRUE;
		nextByteFull = false;
		pulse = true;
		QueryPerformanceCounter(&lastTime);
	}

	else if((data&STREAMZAP_SPACE_MASK)==STREAMZAP_SPACE_MASK) {

		nextByteFull = true;
		pulse = false;

		if(data&STREAMZAP_PULSE_MASK) {
			setData((((data&STREAMZAP_PULSE_MASK)>>4)*STREAMZAP_RESOLUTION) + (STREAMZAP_RESOLUTION/2) | PULSE_BIT);
			SetEvent(dataReadyEvent);
		}
	}
	else if((data&STREAMZAP_PULSE_MASK)==STREAMZAP_PULSE_MASK) {

		//
		// add pulse check its not zero
		//
		if(data&STREAMZAP_PULSE_MASK) {

			nextByteFull = true;
			pulse = true;
		}
	}
	else {

		if(newSignal) {

			//==========
			lirc_t temp;
			//==========

			QueryPerformanceCounter(&time);

			newSignal	= FALSE;
			temp		= (lirc_t)(((time.QuadPart - lastTime.QuadPart)*1000000) / frequency.QuadPart);
			temp	   += 255 * STREAMZAP_RESOLUTION;
			lastTime	= time;

			if(temp>PULSE_MASK) temp = PULSE_MASK;

			setData(temp);
		}
		
		if(nextByteFull) {
			
			//===========
			lirc_t value;
			//===========

			value = (data * STREAMZAP_RESOLUTION) + (STREAMZAP_RESOLUTION/2);

			if(pulse) {
				value |= PULSE_BIT;
			}

			nextByteFull = false;

			setData(value);
			SetEvent(dataReadyEvent);
		}
		else {

			//
			// add pulse
			//
			if(data&STREAMZAP_PULSE_MASK) {
				setData((((data&STREAMZAP_PULSE_MASK)>>4)*STREAMZAP_RESOLUTION) + (STREAMZAP_RESOLUTION/2) | PULSE_BIT);
			}

			//
			// add space
			//
			setData(((data&STREAMZAP_SPACE_MASK)*STREAMZAP_RESOLUTION) + (STREAMZAP_RESOLUTION/2));
			SetEvent(dataReadyEvent);
		}
	}
}

void StreamzapAPI::setData(lirc_t data) {

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool StreamzapAPI::getData(lirc_t *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

void StreamzapAPI::findDevice() {

	//=================================
	HDEVINFO		hardwareDeviceInfo;
	SP_DEVINFO_DATA deviceInfoData;
	//=================================

	hardwareDeviceInfo = SetupDiGetClassDevs(&GUID_CLASS_STREAMZAP,NULL,NULL,(DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)); 

	if (hardwareDeviceInfo == INVALID_HANDLE_VALUE) {
		//printf("Could not find any devices.\n");
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

					SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

					functionClassDeviceData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc (requiredSize);
					memset(functionClassDeviceData, 0, requiredSize);
					functionClassDeviceData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

					if (! SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo,&deviceInterfaceData,functionClassDeviceData,requiredSize,NULL,NULL)) {
						free(functionClassDeviceData);
						//printf("failed here ..");
						break;
					}

					//_tprintf(_T("device name: %s\n"),functionClassDeviceData->DevicePath);

					_tcscpy_s(deviceName,_countof(deviceName),functionClassDeviceData->DevicePath);

					free(functionClassDeviceData);

					return;
				}
				else {
					break;
				}
			}
		}
		else {
			if (ERROR_NO_MORE_ITEMS == GetLastError()) {
				break;
			}
                
			break;	//exit loop here anyway
		}
	}

	 SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
}

