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
//  22Jul04  AS      Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TM_OSDEPEND_EVENT_H
#define TM_OSDEPEND_EVENT_H

#include "tmTypes.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the interface of an OS Depend kernel mode event.
//
//////////////////////////////////////////////////////////////////////////////
class tmIOSDependEvent
{
// Implementation
public:
    //  the destructor needs to be virtual, because otherwise the destructor
    //  of the deriven class never gets called!
    virtual      ~tmIOSDependEvent() {}
    //
    // Description:
    //  The tmSetEvent method sets an event to signaled state if the event 
    //  was not already signaled. If tmSetEvent is followed directly by 
    //  a tmWaitForEvent funtion bWait has to be set true.
    // Return Value:
    //  TM_OK                   The event was set successfully to signaled state.
    //
    virtual UInt32 tmSetEvent( Bool bWait = False ) = 0;
    //
    // Description:
    //  The tmClearEvent method sets the event to not signaled state.
    // Return Value:
    //  TM_OK                   The event was cleared successfully.
    //
    virtual UInt32 tmClearEvent() = 0;
    //
    // Description:
    //  The tmGetEventState method returns if the event is currently in
    //  signaled or not signaled state.
    // Return Value:
    //  TM_ERR_BAD_PARAMETER    A bad input parameter was detected.
    //  TM_OK                   The event state was returned successfully.
    //
    virtual UInt32 tmGetEventState( Bool*  pbSignaled ) = 0;
    //
    // Description:
    //  The tmWaitForEvent routine puts the current thread into a wait state 
    //  until the event object is set to a signaled state. There are two possible
    //  way of waiting, with or without a timeout.
    // Return Value:
    //  TM_ERR_BAD_PARAMETER    A function that is used in this method failed.
    //  TM_ERR_TIMEOUT          A timeout is reached.
    //  TM_OK                   The event state was returned successfully.
    //
    virtual UInt32 tmWaitForEvent( UInt32 dwMilliconds = 0 ) = 0;
};

#endif

