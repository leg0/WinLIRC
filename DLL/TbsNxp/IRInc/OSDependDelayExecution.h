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

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.

#include "tmIOSDependDelayExecution.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the delay execution. It is
//  derived from the virtual class tmIOSDependDelayExecution.
//  This class puts the current thread or the whole system in a wait state
//  or a given time.
//
//////////////////////////////////////////////////////////////////////////////
class tmCOSDependDelayExecution : public tmIOSDependDelayExecution
{
// Construction
public:
            tmCOSDependDelayExecution();
    virtual ~tmCOSDependDelayExecution();

// Implementation
public:
    UInt32 tmDelayExecutionThread(UInt32 dwMilliSec);
    void   tmDelayExecutionSystem(UInt16 wMicroSec );
};
