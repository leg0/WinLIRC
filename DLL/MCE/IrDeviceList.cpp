#include "IrDeviceList.h"
#include <stdio.h>
#include <tchar.h>
#include <initguid.h>

DEFINE_GUID(GUID_CLASS_IRBUS, 0x7951772d, 0xcd50, 0x49b7, 0xb1, 0x03, 0x2b, 0xaa, 0xc4, 0x94, 0xfc, 0x57);

void IrDeviceList::MceIrOpenUsbDevice(LPGUID  pGuid ) {

	//==========================================================
    HANDLE							hOut = INVALID_HANDLE_VALUE;
    HDEVINFO						hardwareDeviceInfo;
	SP_DEVICE_INTERFACE_DATA		deviceInterfaceData;
	//==========================================================

    hardwareDeviceInfo = SetupDiGetClassDevs(pGuid,nullptr,nullptr, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)); 

    if (hardwareDeviceInfo == INVALID_HANDLE_VALUE) {
		printf("Could not find any Mce Ir devices.");
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
				m_List.push_back(deviceInterfaceDetailData->DevicePath);
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


IrDeviceList::IrDeviceList()
{
	MceIrOpenUsbDevice((LPGUID)&GUID_CLASS_IRBUS);
}

IrDeviceList::StringList & IrDeviceList::get()
{
	return m_List;
}