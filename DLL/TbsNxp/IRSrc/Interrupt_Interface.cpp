#include "stdafx.h"
#include <dshow.h>
#include <ks.h>
#include "Interrupt_Interface.h"

#ifndef CheckPointer
	#define CheckPointer(p,ret) {if((p)==NULL) return (ret);}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// Description:         
//
//  Interrupt thread to handle incoming events
//
// Return Value:         
//
//  None
//
//////////////////////////////////////////////////////////////////////////////
DWORD WINAPI InterruptWaitThread( LPVOID pParam ) 
{
	tmInterruptActivateType	tType;
    CInterrupt_Interface* pIntParam = (CInterrupt_Interface*) pParam;

    HANDLE lHandles[3];

    lHandles[0] = pIntParam->m_IntParam[0].Handle;
    lHandles[1] = pIntParam->m_IntParam[1].Handle;
    lHandles[2] = pIntParam->m_IntParamTerminate.Handle;
    
    while(TRUE)
    {
        DWORD dwRet = WaitForMultipleObjects( 3, lHandles, FALSE, INFINITE );

        if( dwRet == 0 )
        {
            tType = FALLING_EDGE_ACTIVATED;
            if ( pIntParam->m_IntParam[0].pCallback )
            {
                pIntParam->m_IntParam[0].pCallback->InterruptCallback( &tType );
            }
        }
        else if( dwRet == 1 )
        {
            tType = RISING_EDGE_ACTIVATED;
            if ( pIntParam->m_IntParam[1].pCallback )
            {
                pIntParam->m_IntParam[1].pCallback->InterruptCallback( &tType );
            }
        }
        else if( dwRet == 2 )
            break;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:         
//
//  Constructor
//
// Return Value:         
//
//  None
//
//////////////////////////////////////////////////////////////////////////////
CInterrupt_Interface::CInterrupt_Interface(IKsControl* pKsDeviceControl, UInt16 wImplementationID)
{
	CoInitialize(NULL);
    // initialize member variables
    m_pKsDeviceControl = pKsDeviceControl;
    m_wImplementationId = wImplementationID;

	// Create the event for the interrupt
    m_IntParam[0].Handle = CreateEvent( NULL, FALSE, FALSE, NULL );
	m_IntParam[1].Handle = CreateEvent( NULL, FALSE, FALSE, NULL );

	// Create the event for the interrupt
    m_IntParamTerminate.Handle = CreateEvent( NULL, FALSE, FALSE, NULL );

	DWORD dwThreadId;
	m_hThread = CreateThread(NULL,0,InterruptWaitThread,this,0,&dwThreadId);
	if (m_hThread) SetThreadPriority(m_hThread,THREAD_PRIORITY_TIME_CRITICAL);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:         
//
//  Destructor
//
// Return Value:         
//
//  None
//
//////////////////////////////////////////////////////////////////////////////
CInterrupt_Interface::~CInterrupt_Interface
(
)
{   
    if ( m_hThread )
       SetEvent( m_IntParamTerminate.Handle );

    CloseHandle( m_IntParam[0].Handle );
    CloseHandle( m_IntParam[1].Handle );

    CloseHandle( m_IntParamTerminate.Handle );
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:         
//
//  Interface function to enable the interrupt
//  Parameter: GPIO number
//             Event type (rising edge or falling edge)
//             Pointer to the upper callback function
//
// Return Value:         
//
//  FALSE if registration failed
//  
//////////////////////////////////////////////////////////////////////////////
UInt32 CInterrupt_Interface::tmIEnableInterrupt( UInt8 ucGPIOPin,
												 tmInterruptActivateType EventType,
												 CInterruptCallback* pCallback )
{
	CheckPointer(TM_ERR_NOT_INITIALIZED, TM_ERR_BAD_PARAMETER);
    CheckPointer(pCallback, TM_ERR_BAD_PARAMETER);

    KSEVENT Request;
	Request.Id    = ucGPIOPin;
	Request.Set   = SAA716x_GPIO_EVENT_SET;
	Request.Flags = KSEVENT_TYPE_ENABLE;

	if ( FALLING_EDGE_ACTIVATED == EventType )
	{
		m_tEventData.EventData.EventHandle.Event = m_IntParam[0].Handle;
		m_IntParam[0].pCallback                  = pCallback;
	}
	else
	{
		m_tEventData.EventData.EventHandle.Event = m_IntParam[1].Handle;
		m_IntParam[1].pCallback                  = pCallback;
	}
	m_tEventData.EventData.NotificationType        = KSEVENTF_EVENT_HANDLE;
	m_tEventData.EventData.EventHandle.Reserved[0] = 0;
	m_tEventData.EventData.EventHandle.Reserved[1] = 0;
	m_tEventData.ucGPIOPin                         = ucGPIOPin;
	m_tEventData.tSignaledState                    = tmFallingEdge;
	//m_tEventData.tSignaledState                  = tmAnyEdge;

	DWORD dwNumBytesReturned;
	HRESULT hr = m_pKsDeviceControl->KsEvent(&Request, sizeof(Request),&m_tEventData,sizeof(m_tEventData),&dwNumBytesReturned);
	return SUCCEEDED(hr) ? TM_OK : TM_ERR_INVALID_COMMAND;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:         
//
//  Function the disable the interrupt
//
// Return Value:         
//
//  Return value of the registration
//
//////////////////////////////////////////////////////////////////////////////
UInt32 CInterrupt_Interface::tmIDisableInterrupt( UInt8 ucGPIOPin, tmInterruptActivateType EventType)
{
	CheckPointer(TM_ERR_NOT_INITIALIZED, TM_ERR_BAD_PARAMETER);
	DWORD dwNumBytesReturned;
	HRESULT hr = m_pKsDeviceControl->KsEvent(NULL, 0, &m_tEventData, sizeof(m_tEventData), &dwNumBytesReturned );
	if ( FALLING_EDGE_ACTIVATED == EventType )
		m_IntParam[0].pCallback = NULL;
	else
		m_IntParam[1].pCallback = NULL;

	return SUCCEEDED(hr) ? TM_OK : TM_ERR_INVALID_COMMAND;
}
