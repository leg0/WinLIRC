#pragma once

#include "tmIGPIOLayer.h"
#include <ksproxy.h>

// {8FDF6765-6DB7-4c9b-860B-2E8DE72D420C}
static const GUID SAA716X_GPIO_ACC_PROPERTY = 
{ 0x8fdf6765, 0x6db7, 0x4c9b, { 0x86, 0xb, 0x2e, 0x8d, 0xe7, 0x2d, 0x42, 0xc } };

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the property set item for GPIO access
//
//////////////////////////////////////////////////////////////////////////////
#define MAIN_GPIO_CONFIG        0
#define MAIN_GPIO_ACCESS        1
#define MAIN_GPIO_GROUP_ACCESS  2

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the GPIO configuration request info structure.
//
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    KSPROPERTY  ksProperty;
    UInt32      dwDeviceType;       // Type enum for the device
    UInt16      wImplementationId;  // Implementation id for the device
    UInt8       ucInstanceId;       // Instance id of this device type
    UInt32      dwGPIOPinMask;      // requested GPIO pins
} tMainGPIOConfigRequest, *ptMainGPIOConfigRequest;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the GPIO configuration data request info structure.
//
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UInt32      dwValue;            //  set/get value for the requested pin
} tMainGPIOConfigData, *ptMainGPIOConfigData;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the GPIO access request info structure.
//
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    KSPROPERTY  ksProperty;
    UInt32      dwDeviceType;      // Type enum for the device
    UInt16      wImplementationId; // Implementation id for the device
    UInt8       ucInstanceId;      // Instance id of this device type
    UInt8       ucGPIOPin;         // requested GPIO pin
} tMainGPIOAccessRequest, *ptMainGPIOAccessRequest;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the GPIO access data request info structure.
//
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UInt8       ucValue;            // set/get value for the requested pin
} tMainGPIOAccessData, *ptMainGPIOAccessData;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the GPIO group access request info structure.
//
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    KSPROPERTY  ksProperty;
    UInt32      dwDeviceType;       // Type enum for the device
    UInt16      wImplementationId;  // Implementation id for the device
    UInt8       ucInstanceId;       // Instance id of this device type    
    UInt32      dwGPIOPinMask;      // requested GPIO pins
} tMainGPIOGroupAccessRequest, *ptMainGPIOGroupAccessRequest;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the GPIO group access data request info structure.
//
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UInt32      dwValue;            // set/get value for the requested pin
} tMainGPIOGroupAccessData, *ptMainGPIOGroupAccessData;
class CGPIO_Interface: public tmIGPIOLayer
{
public:
    CGPIO_Interface( IKsControl* pKsDeviceControl, UInt16 wImplementationID );
    ~CGPIO_Interface();

    UInt32 tmISetPin( UInt8 ucGPIOPin, UInt8 ucWriteValue );
    UInt32 tmIGetPin( UInt8  ucGPIOPin, UInt8* pucGPIOValue );

    UInt32 tmIWriteToGpioGroup( UInt32 dwGpioMask, UInt32 dwValue );
    UInt32 tmIReadFromGpioGroup( UInt32  dwGpioMask, UInt32* pdwValue );

    UInt32 tmISetGpioConfiguration( UInt32  dwGpioMask, UInt32  dwGpioConfig );
    UInt32 tmIGetGpioConfiguration( UInt32* pdwGpioConfig );
private:
	IKsControl*		m_pKsDeviceControl;
    UInt16          m_wImplementationID;
};