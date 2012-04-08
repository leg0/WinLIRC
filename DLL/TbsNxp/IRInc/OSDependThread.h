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

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.

#include "tmIOSDependThread.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the thread. It is derived
//  from the virtual class tmIOSDependThread.
//
//////////////////////////////////////////////////////////////////////////////
class tmCOSDependThread : public tmIOSDependThread
{
// Construction
public:
               tmCOSDependThread();

// Implementation
public:
    virtual    ~tmCOSDependThread();
    virtual UInt32 tmStartThread(pTM_THREAD_START_ROUTINE pStartRoutine,
                                 void* pStartContext);
    virtual UInt32 tmStopThread();

// Attributes
private:
    //thread count
    UInt32   m_dwThreadCount;    
};
