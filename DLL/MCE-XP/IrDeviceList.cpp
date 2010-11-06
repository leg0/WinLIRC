#include "IrDeviceList.h"
#include <stdio.h>

#include <initguid.h>
DEFINE_GUID(GUID_CLASS_IRBUS, 0x7951772d, 0xcd50, 0x49b7, 0xb1, 0x03, 0x2b, 0xaa, 0xc4, 0x94, 0xfc, 0x57);


void IrDeviceList::MceIrOpenOneDevice(HDEVINFO HardwareDeviceInfo, PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData)
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA     functionClassDeviceData = NULL;
    ULONG                                predictedLength = 0;
    ULONG                                requiredLength = 0;
    HANDLE                               hOut = INVALID_HANDLE_VALUE;

    //
    // allocate a function class device data structure to receive the
    // goods about this particular device.
    //
    SetupDiGetDeviceInterfaceDetail (
        HardwareDeviceInfo,
        DeviceInterfaceData,
        NULL, // probing so no output buffer yet
        0, // probing so output buffer length of zero
        &requiredLength,
        NULL); // not interested in the specific dev-node


    predictedLength = requiredLength+16;

    functionClassDeviceData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc (predictedLength);
    memset(functionClassDeviceData, 0, predictedLength);
    //functionClassDeviceData->cbSize = sizeof(PSP_DEVICE_INTERFACE_DETAIL_DATA);
	functionClassDeviceData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    //
    // Retrieve the information from Plug and Play.
    //
    if (! SetupDiGetDeviceInterfaceDetail(
        HardwareDeviceInfo,
        DeviceInterfaceData,
        functionClassDeviceData,
        requiredLength,
        NULL,
        NULL))
    {
        free(functionClassDeviceData);
		printf("Could not open a MceIr device during detection");
        return;
    }

    hOut = CreateFile(
        functionClassDeviceData->DevicePath,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL, // no SECURITY_ATTRIBUTES structure
        OPEN_EXISTING, // No special create flags
        0, // No special attributes
        NULL); // No template file

    if (INVALID_HANDLE_VALUE != hOut)
    {	
		bool found = false;
		std::wstring path = functionClassDeviceData->DevicePath;
		StringList::iterator begin = m_List.begin();
		while ( begin != m_List.end() )
		{
			if ( path == *begin )
			{
				found = true;
				break;
			}

			begin++;
		}

		if ( !found ) 
			m_List.push_back (path);

		CloseHandle(hOut);
    }
	
    free(functionClassDeviceData);

    return;
}

void IrDeviceList::MceIrOpenUsbDevice(LPGUID  pGuid )
{
    HANDLE            hOut = INVALID_HANDLE_VALUE;
    HDEVINFO          hardwareDeviceInfo;
    SP_DEVINFO_DATA   deviceInfoData;

    // Open a handle to the plug and play dev node.
    // SetupDiGetClassDevs() returns a device information set that contains info on all
    // installed devices of a specified class.
    hardwareDeviceInfo = SetupDiGetClassDevs(
        pGuid,
        NULL, // Define no enumerator (global)
        NULL, // Define no
        (DIGCF_PRESENT | // Only Devices present
        DIGCF_DEVICEINTERFACE)); // Function class devices.

    if (hardwareDeviceInfo == INVALID_HANDLE_VALUE)
	{
		printf("Could not find any Mce Ir devices.");
        return;
	}

    for (int i = 0; i< 1000 ; i++)
    {
        memset(&deviceInfoData, 0, sizeof(deviceInfoData));
        deviceInfoData.cbSize = sizeof(deviceInfoData);
        if (SetupDiEnumDeviceInfo (hardwareDeviceInfo, i, &deviceInfoData))
        {
            // SetupDiEnumDeviceInterfaces() returns information about device interfaces
            // exposed by one or more devices. Each call returns information about one interface;
            // the routine can be called repeatedly to get information about several interfaces
            // exposed by one or more devices.
            SP_DEVICE_INTERFACE_DATA   deviceInterfaceData;
            memset(&deviceInterfaceData, 0, sizeof(deviceInterfaceData));
            deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

			for ( int j = 0; j< 1000 ; j++)
			{
				SetupDiEnumDeviceInterfaces (
					hardwareDeviceInfo,
					&deviceInfoData,
					pGuid,
					j,
					&deviceInterfaceData);
				
				MceIrOpenOneDevice(hardwareDeviceInfo, &deviceInterfaceData);
			}			
        }
        else
        {
            if (ERROR_NO_MORE_ITEMS != GetLastError())
			{
				printf("MceIrOpenUsbDevice failed");
                break;
			}
        }
    }

    // SetupDiDestroyDeviceInfoList() destroys a device information set
    // and frees all associated memory.

    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
}


IrDeviceList::IrDeviceList()
{
	// if (MceIrGetDeviceFileName((LPGUID)&GUID_CLASS_IRBUS, DeviceName))
	MceIrOpenUsbDevice((LPGUID)&GUID_CLASS_IRBUS);
}

IrDeviceList::StringList & IrDeviceList::get()
{
	return m_List;
}