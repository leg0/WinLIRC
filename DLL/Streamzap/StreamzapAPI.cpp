#include "StreamzapAPI.h"
#include <stdio.h>
#include "Globals.h"
#include <initguid.h>
#include <setupapi.h>
#include <tchar.h>
#include "../Common/Win32Helpers.h"

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

	m_threadHandle	= NULL;
	m_threadExitEvent	= NULL;

	QueryPerformanceFrequency(&m_frequency);
	QueryPerformanceCounter(&m_lastTime);
}

BOOL StreamzapAPI::init(HANDLE exit) {

	m_exitThread	= false;
	m_nextByteFull	= false;

	m_bufferStart	= 0;
	m_bufferEnd		= 0;
	m_pulse			= PULSE_BIT;
	m_newSignal		= TRUE;
	m_deviceName[0]	= '\0';

	findDevice();

	if(!_tcslen(m_deviceName)) {
		return FALSE;			// failed to find device name
	}

	m_threadExitEvent	= exit;
	m_dataReadyEvent	= CreateEvent(NULL,TRUE,FALSE,NULL);
	m_deviceHandle		= CreateFile(m_deviceName,GENERIC_READ,0,NULL,OPEN_EXISTING,0, NULL); 

	if(m_deviceHandle==NULL) {
		goto init_fail;
	}

	//
	// see if reading works - if not someone else is reading
	//
	{
		//==============
		UCHAR buffer;
		DWORD bytesRead;
		//==============

		if(!ReadFile(m_deviceHandle,&buffer,1,&bytesRead,NULL)) {
			goto init_fail;
		}
	}

	m_threadHandle = CreateThread(NULL,0,SZThread,(void *)this,0,NULL);

	return TRUE;

init_fail:

	SAFE_CLOSE_HANDLE(m_dataReadyEvent);
	SAFE_CLOSE_HANDLE(m_deviceHandle);

	return FALSE;
}

void StreamzapAPI::deinit() {

	m_exitThread = true;

	KillThread(NULL,m_threadHandle);

	SAFE_CLOSE_HANDLE(m_dataReadyEvent);

	m_threadExitEvent = NULL;
}

void StreamzapAPI::threadProc() {

	while(1) {

		//==============
		UCHAR buffer;
		DWORD bytesRead;
		//==============

		ReadFile(m_deviceHandle,&buffer,1,&bytesRead,NULL);
		
		if(bytesRead) {
			//printf("buffer value %i\n",buffer);
			decode(buffer);
		}
		else {
			Sleep(20);
		}

		if(m_exitThread) break;
	}

	//
	// clean up
	//

	SAFE_CLOSE_HANDLE(m_deviceHandle);
}

bool StreamzapAPI::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={m_dataReadyEvent,m_threadExitEvent};
	int evt;
	if(m_threadExitEvent==NULL) evt=1;
	else evt=2;

	if(!dataReady())
	{
		ResetEvent(m_dataReadyEvent);
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

bool StreamzapAPI::dataReady() {

	if(m_bufferStart==m_bufferEnd) return false;
	
	return true;
}

void StreamzapAPI::decode(BYTE data) {

	if(data==STREAMZAP_TIMEOUT) {
		m_newSignal		= TRUE;
		m_nextByteFull	= false;
		m_pulse			= true;
		QueryPerformanceCounter(&m_lastTime);
	}
	else if((data&STREAMZAP_SPACE_MASK)==STREAMZAP_SPACE_MASK) {

		m_nextByteFull	= true;
		m_pulse			= false;

		if(data&STREAMZAP_PULSE_MASK) {
			setData((((data&STREAMZAP_PULSE_MASK)>>4)*STREAMZAP_RESOLUTION) + (STREAMZAP_RESOLUTION/2) | PULSE_BIT);
			SetEvent(m_dataReadyEvent);
		}
	}
	else if((data&STREAMZAP_PULSE_MASK)==STREAMZAP_PULSE_MASK) {

		//
		// add pulse check its not zero
		//
		if(data&STREAMZAP_PULSE_MASK) {
			m_nextByteFull	= true;
			m_pulse			= true;
		}
	}
	else {

		if(m_newSignal) {

			//==========
			lirc_t temp;
			//==========

			QueryPerformanceCounter(&m_time);

			m_newSignal	= FALSE;
			temp		= (lirc_t)(((m_time.QuadPart - m_lastTime.QuadPart)*1000000) / m_frequency.QuadPart);
			temp	   += 255 * STREAMZAP_RESOLUTION;
			m_lastTime	= m_time;

			if(temp>PULSE_MASK) temp = PULSE_MASK;

			setData(temp);
		}
		
		if(m_nextByteFull) {
			
			//===========
			lirc_t value;
			//===========

			value = (data * STREAMZAP_RESOLUTION) + (STREAMZAP_RESOLUTION/2);

			if(m_pulse) {
				value |= PULSE_BIT;
			}

			m_nextByteFull = false;

			setData(value);
			SetEvent(m_dataReadyEvent);
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
			SetEvent(m_dataReadyEvent);
		}
	}
}

void StreamzapAPI::setData(lirc_t data) {

	m_dataBuffer[m_bufferEnd] = data;
	m_bufferEnd++;
}

bool StreamzapAPI::getData(lirc_t *out) {

	if(!dataReady()) return false;

	*out = m_dataBuffer[m_bufferStart];

	m_bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

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

					_tcscpy_s(m_deviceName,_countof(m_deviceName),functionClassDeviceData->DevicePath);

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

