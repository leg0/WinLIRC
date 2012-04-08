//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2005
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
//  04Apr05  PA      Created
//
//////////////////////////////////////////////////////////////////////////////

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.
#include "tmIOSDependEvent.h"
//#include "34AVStrm.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the OS depend kernel mode event.
//
//////////////////////////////////////////////////////////////////////////////
class tmCOSDependEvent: public tmIOSDependEvent
{
// Construction
public:
                   tmCOSDependEvent();

// Implementation
public:
    virtual        ~tmCOSDependEvent();
    virtual UInt32 tmSetEvent( Bool bWait );
    virtual UInt32 tmClearEvent();
    virtual UInt32 tmGetEventState( Bool*  pbSignaled );
    virtual UInt32 tmWaitForEvent( UInt32  dwMilliseconds );

// Attributes
private:
    //event object
};


