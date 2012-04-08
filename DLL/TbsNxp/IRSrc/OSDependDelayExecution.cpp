//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2004
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  Philips reserves the right to make changes without notice at any time.
//  Philips makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. Philips must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// Modification History:
//
//  Date     By      Description
//  -------  ------  ---------------------------------------------------------
//  20Oct04  CP      Created
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tmIOSDependFactory.h"
#include "OSDependDelayExecution.h"
#include <stdio.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor
//
//////////////////////////////////////////////////////////////////////////////
tmCOSDependDelayExecution::tmCOSDependDelayExecution()
{    
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor
//
//////////////////////////////////////////////////////////////////////////////
tmCOSDependDelayExecution::~tmCOSDependDelayExecution()
{    
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This method blocks the execution of the current thread for the required
//  time in milliseconds. The precision is better than 20 milliseconds.
//  Can only be called at IRQL = PASSIVE_LEVEL
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
UInt32 tmCOSDependDelayExecution::tmDelayExecutionThread
(
    UInt32 dwMilliSec    // Required time in units of milliseconds.
                         // The value must be within the range of 1 to 4000.
)
{
    // check if parameter is within valid range
    if( (dwMilliSec < 1) || (dwMilliSec > 4000) )
    {
        tmDBGPRINTEx(0,("Error: tmDelayExecutionThread Parameter out of range"));
        return TM_OK;
    }

    Sleep(dwMilliSec);
    
//    tmosalTaskSleep( dwMilliSec );

    return TM_OK;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This method stops the CPU activity for the required time.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void tmCOSDependDelayExecution::tmDelayExecutionSystem
(
    UInt16 wMicroSec  // Microseconds to stop the CPU activities,
                      // The value must be within the range of 1 to 50.
)
{
    // check if parameter is within valid range
    if( ( wMicroSec < 1 ) || ( wMicroSec > 50 ) )
    {
        tmDBGPRINTEx(0,("Error: Parameter out of range"));
        return;
    }

    Sleep( wMicroSec );
}

