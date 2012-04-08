//////////////////////////////////////////////////////////////////////////////
//
//                     (C) NXP Semiconductors 2000-2007
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
//////////////////////////////////////////////////////////////////////////////
//
// Modification History:
//
//  Date     By      Description
//  -------  ------  ---------------------------------------------------------
//  12Jun03  ML      Created
//
//////////////////////////////////////////////////////////////////////////////

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.

#include "tmIOSDependTimeOut.h"

#define TM_MAX_TIMER 20

typedef struct
{
    tmIOSDependAbstractCB* pCallbackHandler;
    Bool                   bPeriodic;
    UINT_PTR               pHandle;
} tmTimerData_t;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the time out. It is derived
//  from the virtual class COSDependTimeOut.
//
//////////////////////////////////////////////////////////////////////////////
class tmCOSDependTimeOut : public tmIOSDependTimeOut
{
public:
    //  the destructor needs to be virtual, because otherwise the destructor
    //  of the deriven class never gets called!
            tmCOSDependTimeOut();
    virtual ~tmCOSDependTimeOut();
    //  this function initializes the requested time out
    // Parameters:
    //  dwTimeToWait   - time to wait in milliseconds
    //  bPeriodic      - should time out be periodic or just a single event
    //  pAbstractCBObj - callback class, contains the callback handler
    // Return Value:
    //  TRUE   initialization was successful
    //  FALSE  initialization was unsuccessful
    virtual Bool InitializeTimeOut( UInt32                 dwTimeToWait,
                                    Bool                   bPeriodic,
                                    tmIOSDependAbstractCB* pAbstractCBObj );
    virtual void tmQueryPerformanceFrequency( __int64* pFreq );
    virtual void tmQueryPerformanceCounter( __int64* pCounter );
    virtual void tmQueryPerformanceTime( __int64* pCounter );
    virtual void tmQueryTimeIncrement( __int64* pCounter );

    Bool    GetObjectStatus();
private:
    Bool          m_bObjectStatus;//status of object creation
    __int64       m_Freq;
    LARGE_INTEGER m_llFreq;
};
