#include "stdafx.h"
#include <dshow.h>
#include <ks.h>
#include "GPIO_Interface.h"

// constructor
CGPIO_Interface::CGPIO_Interface(IKsControl* pKsDeviceControl, UInt16 wImplementationID)
{
    // initialize member variables
    m_pKsDeviceControl = pKsDeviceControl;
    m_wImplementationID = wImplementationID;
}

// destructor
CGPIO_Interface::~CGPIO_Interface()
{
}

// method to set value to specified GPIO pin
UInt32 CGPIO_Interface::tmISetPin( UInt8 ucGPIOPin, UInt8 ucWriteValue)
{
	tMainGPIOAccessRequest   Request;
	
	Request.ksProperty.Set    = SAA716X_GPIO_ACC_PROPERTY;
	Request.ksProperty.Id     = MAIN_GPIO_ACCESS;
	Request.ksProperty.Flags  = KSPROPERTY_TYPE_SET;
	Request.ucGPIOPin         = ucGPIOPin;
	Request.dwDeviceType      = 0x2; // gpio source
	Request.ucInstanceId      = 1;
	Request.wImplementationId = m_wImplementationID;

	DWORD dwNumBytesReturned;
	HRESULT hr = m_pKsDeviceControl->KsProperty( (PKSPROPERTY)&Request, sizeof(Request),
											&ucWriteValue, sizeof(tMainGPIOAccessData),
											&dwNumBytesReturned);
	return SUCCEEDED(hr) ? TM_OK : TM_ERR_INVALID_COMMAND;
}

// method to read out specified GPIO pin
UInt32 CGPIO_Interface::tmIGetPin( UInt8 ucGPIOPin, UInt8* pucGPIOValue)
{
	tMainGPIOAccessRequest   Request;
	
	Request.ksProperty.Set    = SAA716X_GPIO_ACC_PROPERTY;
	Request.ksProperty.Id     = MAIN_GPIO_ACCESS;
	Request.ksProperty.Flags  = KSPROPERTY_TYPE_GET;
	Request.ucGPIOPin         = ucGPIOPin;
	Request.dwDeviceType      = 0x2; // gpio source
	Request.ucInstanceId      = 1;
	Request.wImplementationId = m_wImplementationID;

	DWORD dwNumBytesReturned;
	HRESULT hr = m_pKsDeviceControl->KsProperty( (PKSPROPERTY)&Request, sizeof(Request),
											pucGPIOValue, sizeof(tMainGPIOAccessData),
											&dwNumBytesReturned);
	return SUCCEEDED(hr) ? TM_OK : TM_ERR_INVALID_COMMAND;
}

UInt32 CGPIO_Interface::tmIGetGpioConfiguration( UInt32* pdwGpioConfig )
{
	tMainGPIOConfigRequest   Request;
	
	Request.ksProperty.Set    = SAA716X_GPIO_ACC_PROPERTY;
	Request.ksProperty.Id     = MAIN_GPIO_CONFIG;
	Request.ksProperty.Flags  = KSPROPERTY_TYPE_GET;
	Request.dwGPIOPinMask     = 0;
	Request.dwDeviceType      = 0x2; // gpio source
	Request.ucInstanceId      = 1;
	Request.wImplementationId = m_wImplementationID;

	DWORD dwNumBytesReturned;
	HRESULT hr = m_pKsDeviceControl->KsProperty( (PKSPROPERTY)&Request, sizeof(Request),
											pdwGpioConfig, sizeof(tMainGPIOConfigData),
											&dwNumBytesReturned);
	return SUCCEEDED(hr) ? TM_OK : TM_ERR_INVALID_COMMAND;
}

UInt32 CGPIO_Interface::tmISetGpioConfiguration( UInt32 dwGpioMask, UInt32 dwGpioConfig )
{
	tMainGPIOConfigRequest   Request;
	
	Request.ksProperty.Set    = SAA716X_GPIO_ACC_PROPERTY;
	Request.ksProperty.Id     = MAIN_GPIO_CONFIG;
	Request.ksProperty.Flags  = KSPROPERTY_TYPE_SET;
	Request.dwGPIOPinMask     = dwGpioMask;
	Request.dwDeviceType      = 0x2; // gpio source
	Request.ucInstanceId      = 1;
	Request.wImplementationId = m_wImplementationID;

	DWORD dwNumBytesReturned;
	HRESULT hr = m_pKsDeviceControl->KsProperty( (PKSPROPERTY)&Request, sizeof(Request),
											&dwGpioConfig, sizeof(tMainGPIOConfigData),
											&dwNumBytesReturned);
	return SUCCEEDED(hr) ? TM_OK : TM_ERR_INVALID_COMMAND;
}
