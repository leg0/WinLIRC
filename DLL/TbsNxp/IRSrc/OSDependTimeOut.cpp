//////////////////////////////////////////////////////////////////////////////
//
//                     (C) NXP Semiconductors 2006-2007
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  NXP reserves the right to make changes without notice at any time.
//  NXP makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. NXP must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifndef IN
#define IN
#endif

#include "Windows.h"
#include "tmIOSDependFactory.h"
#include "OSDependTimeOut.h"

#ifndef SIZEOF_ARRAY
    #define SIZEOF_ARRAY(ar)        (sizeof(ar)/sizeof((ar)[0]))
#endif // !defined(SIZEOF_ARRAY)

#define NANOSECONDS 10000000
#define CONVERT_PERFORMANCE_TIME( Frequency, PerformanceTime )                                \
    ((( (ULONGLONG)(ULONG)(PerformanceTime).HighPart * NANOSECONDS / (Frequency)) << 32) +    \
    ((((( (ULONGLONG)(ULONG)(PerformanceTime).HighPart * NANOSECONDS) % (Frequency)) << 32) + \
    ( (ULONGLONG)(PerformanceTime).LowPart * NANOSECONDS)) / (Frequency)))

tmTimerData_t g_tTimerInUse[TM_MAX_TIMER];

void __stdcall TimerCallback
(
    HWND     hwnd,
    UINT     nEvent,
    UINT_PTR pnData,
    DWORD    dwData
)
{
    // Find the corresponding slot
    for ( UInt8 i = 0; i < TM_MAX_TIMER; i++ )
    {
        if ( pnData == g_tTimerInUse[i].pHandle )
        {
            if ( g_tTimerInUse[i].pCallbackHandler )
            {
                g_tTimerInUse[i].pCallbackHandler->CallbackHandler();
            }
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor
//
//////////////////////////////////////////////////////////////////////////////
tmCOSDependTimeOut::tmCOSDependTimeOut()
{
    m_bObjectStatus = TM_OK;

    memset( &g_tTimerInUse, 0, SIZEOF_ARRAY(g_tTimerInUse) );

    QueryPerformanceFrequency( &m_llFreq );
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor
//
//////////////////////////////////////////////////////////////////////////////
tmCOSDependTimeOut::~tmCOSDependTimeOut()
{
    m_bObjectStatus = TM_ERR_NOT_INITIALIZED;

    for ( UInt8 i = 0; i < TM_MAX_TIMER; i++)
    {
        if ( g_tTimerInUse[i].pHandle )
        {
            KillTimer( 0 , g_tTimerInUse[i].pHandle  );
            g_tTimerInUse[i].pHandle = NULL;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates and Initializes the time out object. If there is a time out object
//  that was created before the old time out object will be deleted.
//  The handler of the abstract callback class is called after the period of
//  time (in milliseconds). If the bPeriodic flag is TRUE, the time out object
//  will be set to recurrent mode. The time for the period is equal to
//  dwTimeToWait. Time must be between 50 and 2000 msec. Caller must be
//  running at passive level.
//
// Return Value:
//  FALSE       Memory cannot be allocated. For more details see description
//              of operator new.
//  TRUE        Created the time out object with success.
//
//////////////////////////////////////////////////////////////////////////////
Bool tmCOSDependTimeOut::InitializeTimeOut
(
    UInt32 dwTimeToWait, //Specifies the absolute time at which the timer is to
                        //expire. The expiration time is expressed in system
                        //time units (one milliecond intervals). Absolute
                        //expiration times track any changes in the system
                        //time. Time must be between 50 and 2000 msec.
    Bool bPeriodic,     //Set the time out object to recurrent mode. The time
                        //for the period is equal to dwTimeToWait.
    tmIOSDependAbstractCB* pAbstractCBObj //Pointer to callback object.
)
{
    // check if parameter is valid
    if( !pAbstractCBObj )
    {
        tmDBGPRINTEx(DEBUGLVL_ERROR,("Error: Callback object invalid"));
        return False;
    }

    UINT_PTR pStatus = SetTimer( 0, (UINT_PTR)pAbstractCBObj, 
                                 dwTimeToWait, TimerCallback );
    if ( pStatus )
    {
        // Search for an empty slot
        for ( UInt8 i = 0; i < TM_MAX_TIMER; i++)
        {
            if ( NULL == g_tTimerInUse[i].pHandle )
            {
                g_tTimerInUse[i].pHandle          = pStatus;
                g_tTimerInUse[i].pCallbackHandler = pAbstractCBObj;
                g_tTimerInUse[i].bPeriodic        = bPeriodic;
                break;
            }
        }
    }
    else
    {
        tmDBGPRINTEx(DEBUGLVL_ERROR,("Error: SetTimer failed"));
    }

/*
    UINT_PTR pStatus = SetTimer( 0, (UINT_PTR)pAbstractCBObj, 
                                 dwTimeToWait, TimerCallback );

    if ( pStatus )
    {
        // Search for an empty slot
        for ( UInt8 i = 0; i < TM_MAX_TIMER; i++)
        {
            if ( NULL == g_tTimerInUse[i].pHandle )
            {
                g_tTimerInUse[i].pHandle          = pStatus;
                g_tTimerInUse[i].pCallbackHandler = pAbstractCBObj;
                g_tTimerInUse[i].bPeriodic        = bPeriodic;
                break;
            }
        }
    }
    else
    {
        tmDBGPRINTEx(DEBUGLVL_ERROR,("Error: SetTimer failed"));
    }
*/
    return True;
}

void tmCOSDependTimeOut::tmQueryPerformanceFrequency
(
    __int64* pFreq 
)
{
    if ( NULL == pFreq )
    {
        tmDBGPRINTEx(DEBUGLVL_ERROR,("Error: Invalid parameter"));
        return;
    }

    LARGE_INTEGER llFreq;

    QueryPerformanceFrequency( &llFreq );

    *pFreq = llFreq.QuadPart;
}

void tmCOSDependTimeOut::tmQueryPerformanceCounter
(
    __int64* pllCounter
)
{
    if ( NULL == pllCounter )
    {
        tmDBGPRINTEx(DEBUGLVL_ERROR,("Error: Invalid parameter"));
        return;
    }

    LARGE_INTEGER Counter;

    QueryPerformanceCounter( &Counter );

    *pllCounter = Counter.QuadPart;
}

void tmCOSDependTimeOut::tmQueryPerformanceTime
(
    __int64* pllCounter
)
{
    if ( NULL == pllCounter )
    {
        tmDBGPRINTEx(DEBUGLVL_ERROR,("Error: Invalid parameter"));
        return;
    }

    LARGE_INTEGER Counter;

    QueryPerformanceCounter( &Counter );

    *pllCounter = CONVERT_PERFORMANCE_TIME( m_llFreq.QuadPart, Counter );
}

void tmCOSDependTimeOut::tmQueryTimeIncrement
(
    __int64* pllCounter
)
{
    if ( NULL == pllCounter )
    {
        tmDBGPRINTEx(DEBUGLVL_ERROR,("Error: Invalid parameter"));
        return;
    }
}