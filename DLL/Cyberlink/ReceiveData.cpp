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
 * Copyright (C) 2013 Jan Dubiec <jdx(at)onet(dot)pl>
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <setupapi.h>
#include "Globals.h"
#include "ReceiveData.h"
#include "../Common/DebugOutput.h"
#include "../Common/Win32Helpers.h"

#define CLMAKECODE(a, b, c, d) ( (a)<<24 | (b)<<16 | (c)<<8 | (d) )
#define CLBADCODE	0xFFFFFFFF

static const GUID CYBERLINK_RC_DEVICE = { 0xFE050E98, 0x31CD, 0x47EA, { 0xAC, 0x39, 0xCB, 0x14, 0x3E, 0xF2, 0x08, 0xB2 } };

DWORD WINAPI CyberLinkReaderThread(void *threadParameter)
{
	receiveData->threadProc((int)threadParameter);

	return 0;
}

ReceiveData::ReceiveData()
{
	bufferStart				= 0;
	bufferEnd				= 0;
	dataBufferSemaphore		= nullptr;

	nDevices				= 0;

	for ( int i = 0; i < 2; i++ ) {
		deviceHandle[i]		= nullptr;
		usbHandle[i]		= INVALID_HANDLE_VALUE;
		inPipeId[i]			= 0;
		inMaxPacketSize[i]	= 0;
		exitEvent[i]		= nullptr;
	}
}

void ReceiveData::findCyberLinkDevicePaths(LPGUID  pGuid )
{
	//==========================================================
    HANDLE							hOut = INVALID_HANDLE_VALUE;
    HDEVINFO						hardwareDeviceInfo;
	SP_DEVICE_INTERFACE_DATA		deviceInterfaceData;
	//==========================================================

    hardwareDeviceInfo = SetupDiGetClassDevs(pGuid, nullptr, nullptr, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)); 

    if (hardwareDeviceInfo == INVALID_HANDLE_VALUE) {
		DPRINTF("Could not find any CyberLink remote controller.");
        return;
	}
 
    memset(&deviceInterfaceData, 0, sizeof(deviceInterfaceData));
    deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

	for ( int i = 0; ; i++)
	{
		if(SetupDiEnumDeviceInterfaces (hardwareDeviceInfo, nullptr, pGuid, i, &deviceInterfaceData)) {

			//================================================================
			DWORD requiredLength;
			PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData = nullptr;
			//================================================================

			SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo, &deviceInterfaceData, nullptr, 0, &requiredLength, nullptr);

			deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new UCHAR[requiredLength];
			deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			
			if(SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo,&deviceInterfaceData,deviceInterfaceDetailData,requiredLength,nullptr,nullptr)) {
				CyberLinkDevicePaths.push_back(deviceInterfaceDetailData->DevicePath);
			}

			delete [] deviceInterfaceDetailData;

		}
		else {
			break;
		}
	}			

    // SetupDiDestroyDeviceInfoList() destroys a device information set
    // and frees all associated memory.
    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
}

bool ReceiveData::init()
{
	//====================
	std::wstring pipeName;
	USB_INTERFACE_DESCRIPTOR ifaceDescriptor;
	WINUSB_PIPE_INFORMATION pipeInfo;
	//====================

	end = std::chrono::steady_clock::now();

	findCyberLinkDevicePaths((LPGUID)&CYBERLINK_RC_DEVICE); 

	if ( CyberLinkDevicePaths.empty() ) return false;		// no hardware

	nDevices = (int)CyberLinkDevicePaths.size();
	if ( nDevices != 2 ) return false;	// strange, something must be wrong

	dataBufferSemaphore = CreateSemaphore(nullptr, 1, 1, nullptr);

	for (int i = 0; i < nDevices; i++) {
		pipeName = CyberLinkDevicePaths[i].c_str();

		deviceHandle[i] = CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);

		if ( deviceHandle[i] == INVALID_HANDLE_VALUE ) {
			DPRINTF("Invalid device handle\n");
			return false;
		}

		if ( WinUsb_Initialize(deviceHandle[i], &(usbHandle[i]) ) == 0 ) {
			DPRINTF("WinUsb_Initialize failed, error - %ld\n", GetLastError());
			return false;
		}

		if ( WinUsb_QueryInterfaceSettings(usbHandle[i], 0, &ifaceDescriptor) == 0 ) {
			DPRINTF("WinUsb_QueryInterfaceSettings failed, error - %ld\n", GetLastError());
			return false;
		}

		inPipeId[i] = 0;
		for (int j = 0; j < ifaceDescriptor.bNumEndpoints; j++) {
			if ( WinUsb_QueryPipe(usbHandle[i], 0, (UCHAR)j, &pipeInfo) == 0 ) {
				DPRINTF("WinUsb_QueryPipe failed, error - %ld\n", GetLastError());
				return false;
			}

			if ( USB_ENDPOINT_DIRECTION_IN(pipeInfo.PipeId) ) {
				inPipeId[i] = pipeInfo.PipeId;
				inMaxPacketSize[i] = pipeInfo.MaximumPacketSize;
			}
		}
		if (inPipeId[i] == 0)
			return false;

		threadHandle[i] = CreateThread(nullptr, 0, CyberLinkReaderThread, (void *)i, 0, nullptr);

		if( !threadHandle[i] ) {
			return false;
		}
	}

	return true;
}

void ReceiveData::deinit()
{
	for ( int i = 0; i < nDevices; i++ ) {
		KillThread(exitEvent[i],threadHandle[i]);
	}

	SAFE_CLOSE_HANDLE(dataBufferSemaphore);

	for ( int i = 0; i < nDevices; i++ ) {

		if ( usbHandle[i] ) {
			WinUsb_Free(usbHandle[i]);
			usbHandle[i] = nullptr;
		}

		SAFE_CLOSE_HANDLE(exitEvent[i]);
		SAFE_CLOSE_HANDLE(deviceHandle[i]);
	}
}

void ReceiveData::threadProc(int threadNumber)
{
	//========================
	OVERLAPPED	overlappedRead;
	HANDLE		events[2];
	DWORD		result;
	DWORD		bytesRead, bytesReadDummy;
	DWORD		code;
	UCHAR		buffer[32];
	//========================

	memset(&overlappedRead, 0 ,sizeof(OVERLAPPED));

	overlappedRead.hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	exitEvent[threadNumber] = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	events[0] = overlappedRead.hEvent;
	events[1] = exitEvent[threadNumber];

	while (true) {

		WinUsb_ReadPipe(usbHandle[threadNumber], inPipeId[threadNumber], buffer, inMaxPacketSize[threadNumber], &bytesReadDummy, &overlappedRead);

		result = WaitForMultipleObjects(2, events, FALSE, INFINITE);

		if ( result == WAIT_OBJECT_0 ) {

			start = std::chrono::steady_clock::now();

			WinUsb_GetOverlappedResult(usbHandle[threadNumber], &overlappedRead, &bytesRead, FALSE);

			if ( (bytesRead == 8) || (bytesRead == 4) ) {
				code = CLMAKECODE(buffer[0], buffer[1], buffer[2], buffer[3]);
			} else if ( bytesRead == 3 ) {
				code = CLMAKECODE(buffer[0], buffer[1], buffer[2], 0);
			} else if ( bytesRead == 2 ) {
				code = CLMAKECODE(buffer[0], buffer[1], 0, 0);
			} else {
				// an error
				code = CLBADCODE;
				//continue;
			}

			last = end;

			end = std::chrono::steady_clock::now();

			setData(code);
		} else {
			CancelIo(deviceHandle[threadNumber]);
			break;
		}
	}
}

bool ReceiveData::waitTillDataIsReady(std::chrono::microseconds maxUSecs)
{
	HANDLE events[2] = { dataReadyEvent, threadExitEvent };
	int evt;

	if ( threadExitEvent == nullptr)
		evt = 1;
	else
		evt = 2;

	if ( !dataReady() ) {
		ResetEvent(dataReadyEvent);
		using namespace std::chrono;
		DWORD const dwTimeout = maxUSecs > 0us
			? duration_cast<milliseconds>(maxUSecs + 500us).count()
			: INFINITE;
		DWORD const res = ::WaitForMultipleObjects(2, events, false, dwTimeout);
		if ( res == (WAIT_OBJECT_0 + 1) ) {
			return false;
		}
	}

	return true;
}

void ReceiveData::setData(lirc_t data)
{
	DWORD res;

	res = WaitForSingleObject(dataBufferSemaphore, INFINITE);
	if ( res == WAIT_OBJECT_0 ) {
		dataBuffer[bufferEnd] = data;
		bufferEnd++;
	} else {
		// an error
	}
	ReleaseSemaphore(dataBufferSemaphore, 1, nullptr);
	SetEvent(dataReadyEvent);
}

bool ReceiveData::dataReady()
{
	DWORD res;
	bool retval = true;

	res = WaitForSingleObject(dataBufferSemaphore, INFINITE);
	if ( res == WAIT_OBJECT_0 ) {
		if ( bufferStart == bufferEnd) {
			retval = false;
		}
	} else {
		// an error
	}
	ReleaseSemaphore(dataBufferSemaphore, 1, nullptr);

	return retval;
}

bool ReceiveData::getData(lirc_t *out)
{
	DWORD res;

	if ( !dataReady() )
		return false;

	res = WaitForSingleObject(dataBufferSemaphore, INFINITE);
	if ( res == WAIT_OBJECT_0 ) {
		*out = dataBuffer[bufferStart];
		bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want
	} else {
		// an error
	}
	ReleaseSemaphore(dataBufferSemaphore, 1, nullptr);

	return true;
}
