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
//  22May03  CP      Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TM_OSDEPEND_TIMEOUT_H
#define TM_OSDEPEND_TIMEOUT_H

#include "tmTypes.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This is an abstract callback class which can be used to install a user 
//  defined handler into any asynchronous event class 
//  (such as COSDependTimeOut). Just derive your class from this one and 
//  implement the handler. Then post it into the event class.
//
//////////////////////////////////////////////////////////////////////////////
class tmIOSDependAbstractCB
{
public:
    //callback handler, gets called from any asyncronous event class
    virtual void CallbackHandler() = 0;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class provides access to a specified timer.
//
//////////////////////////////////////////////////////////////////////////////
class tmIOSDependTimeOut
{
public:
    //  the destructor needs to be virtual, because otherwise the destructor
    //  of the deriven class never gets called!
    virtual ~tmIOSDependTimeOut(){};
    //  this function initializes the requested time out
    // Parameters:
    //  dwTimeToWait   - time to wait in milliseconds
    //  bPeriodic      - should time out be periodic or just a single event
    //  pAbstractCBObj - callback class, contains the callback handler
    // Return Value:
    //  TRUE   initialization was successful
    //  FALSE  initialization was unsuccessful
    virtual Bool InitializeTimeOut(
        UInt32                 dwTimeToWait,
        Bool                   bPeriodic,
        tmIOSDependAbstractCB* pAbstractCBObj ) = 0;

    virtual void tmQueryPerformanceFrequency( __int64* pFreq ) = 0;
    virtual void tmQueryPerformanceCounter( __int64* pCounter ) = 0;
    virtual void tmQueryPerformanceTime( __int64* pCounter ) = 0;
    virtual void tmQueryTimeIncrement( __int64* pCounter ) = 0;
};

#endif

