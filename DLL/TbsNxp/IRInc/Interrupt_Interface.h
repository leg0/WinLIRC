#pragma once

#include "tmIInterruptLayer.h"
#include "ksproxy.h"

// {8E3161FC-D1A6-41e0-AFEE-3650EF834659}
static const GUID SAA716x_GPIO_EVENT_SET = 
{ 0x8e3161fc, 0xd1a6, 0x41e0, { 0xaf, 0xee, 0x36, 0x50, 0xef, 0x83, 0x46, 0x59 } };

/////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The enumeration describes the signaled state of an interrupt or 
//  event source.
//
// Settings:
//    tmFallingEdge   - Signaled with falling edge.
//    tmRisingEdge    - Signaled with rising edge.
//    tmAnyEdge       - Signaled with any edge.
//
/////////////////////////////////////////////////////////////////////////////
typedef enum
{
    tmFallingEdge   = 0x00,
    tmRisingEdge    = 0x01,
    tmAnyEdge       = 0x02
} tmSignaledState_t, *ptmSignaledState_t;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the GPIO event data structure.
//
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    KSEVENTDATA       EventData;
    UInt8             ucGPIOPin;
    tmSignaledState_t tSignaledState;
} tmGPIOEventData_t, *ptmGPIOEventData_t;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the GPIO callback request info structure.
//
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    KSEVENT ksEvent;
	BYTE ucGPIOPin;
} tGPIOCallbackRequest, *ptGPIOCallbackRequest;


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the GPIO event data info structure.
//
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
	KSEVENTDATA EventData; 
} tGPIOEventData, *ptGPIOEventData;

typedef struct
{
    HANDLE              Handle;
	CInterruptCallback* pCallback;
} tInterruptThreadParam;

enum eInterruptEnable
{
    EN_GPIO_23_NEG = 0,  
    EN_GPIO_23_POS,
    EN_GPIO_22_NEG,
    EN_GPIO_22_POS,
    EN_GPIO_18_NEG,
    EN_GPIO_18_POS,
    EN_GPIO_16_NEG,
    EN_GPIO_16_POS
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:         
//
//  Interrupt class
//
//////////////////////////////////////////////////////////////////////////////
class CInterrupt_Interface: public tmIInterruptLayer
{
public:
    CInterrupt_Interface( IKsControl* pKsDeviceControl, WORD wImplementationId );
    ~CInterrupt_Interface();

    UInt32 tmIEnableInterrupt( UInt8 ucGPIOPin, tmInterruptActivateType EventType, CInterruptCallback* pCallback );
    UInt32 tmIDisableInterrupt( UInt8 ucGPIOPin, tmInterruptActivateType EventType );

	tInterruptThreadParam m_IntParam[2];
    tInterruptThreadParam m_IntParamTerminate;
private:
    tGPIOEventData        m_Data;
    tmGPIOEventData_t     m_tEventData;
    HANDLE                m_hThread;

	IKsControl*		m_pKsDeviceControl;
	WORD			m_wImplementationId;
};
