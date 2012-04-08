//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2003
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
//  26Feb03  AW      Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TM_OSDEPEND_DELAY_EXECUTION_H
#define TM_OSDEPEND_DELAY_EXECUTION_H

#include "tmTypes.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class puts the current thread or the whole system in a wait state 
//  for a given time.
//
//////////////////////////////////////////////////////////////////////////////
class tmIOSDependDelayExecution
{
public:
    //  the destructor needs to be virtual, because otherwise the destructor 
    //  of the deriven class never gets called!
    virtual ~tmIOSDependDelayExecution(){};
    //  This method blocks the execution of the current thread for the 
    //  required time in milliseconds. The precision is better than 20 
    //  milliseconds.
    // Parameters:
    //  dwMilliSec - required time in units of milliseconds. The value must 
    //               be in the range of 1 to 4000.
    virtual UInt32 tmDelayExecutionThread(UInt32 dwMilliSec) = 0;

    //  This method stops the CPU activity for the required time in real 
    //  time clock ticks (0.8 microseconds). The precision is better than 2.4
    //  microseconds. Be careful, Drivers that call this routine should 
    //  minimize the number of microseconds they specify (no more than 50).
    //  If a driver must wait for a longer interval, it should use another 
    //  synchronization mechanism.
    // Parameters:
    //  wMicroSec - microseconds to stop the CPU activities. The value 
    //              must be within the range of 1 to 50.
    virtual void tmDelayExecutionSystem(UInt16 wMicroSec) = 0;
};

#endif

