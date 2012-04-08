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

#ifndef TM_OSDEPEND_THREAD_H
#define TM_OSDEPEND_THREAD_H

#include "tmTypes.h"

typedef UInt8 (*pTM_THREAD_START_ROUTINE) (IN void* ThreadContext);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the OS Depend kernel mode thread.
//
//////////////////////////////////////////////////////////////////////////////
class tmIOSDependThread
{
// Implementation
public:
    //  the destructor needs to be virtual, because otherwise the destructor
    //  of the deriven class never gets called!
    virtual      ~tmIOSDependThread() {}
    // Description:
    //  The tmStartThread method creates and starts a system thread that 
    //  executes in kernel mode.
    //  NOTE: For Windows 2000 and Windows 98/Me must only call from the 
    //        system process context.
    // Parameters:
    //  pStartRoutine - The entry point for the thread
    //  pStartContext - Supplies a single argument passed to the thread when 
    //                  it begins execution.
    // Return Value:
    //  TM_ERR_NOT_SUPPORTED   The function is not implemented.
    //  TM_ERR_BAD_PARAMETER   One or both input params are invalid.
    //  TM_ERR_FUNC_FAILED     A function that is used in this method failed.      

    virtual UInt32 tmStartThread(pTM_THREAD_START_ROUTINE pStartRoutine,
                                 void* pStartContext) = 0;
    // Description:
    //  The tmStopThread method stops and terminates the thread.
    //  NOTE: tmStopThread must be called in the context of the thread; 
    //        that is, the tmStartThread created thread must terminate 
    //        itself by making this call.
    // Return Value:
    //  TM_ERR_NOT_SUPPORTED   The function is not implemented.
    //  TM_ERR_FUNC_FAILED     A function that is used in this method failed.      
    virtual UInt32 tmStopThread() = 0;
};

#endif

