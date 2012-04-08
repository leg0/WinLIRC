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
//  11JUL04  AW      Created
//
//////////////////////////////////////////////////////////////////////////////

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.

#include "34AVStrm.h"
#include "tmIOSDependInterruptSafeCall.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the sync call. It is derived
//  from the virtual class COSDependSyncCall.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependInterruptSafeCall : public tmIOSDependInterruptSafeCall
{
// Construction
public:
                COSDependInterruptSafeCall(PKINTERRUPT pIntObject);
    virtual     ~COSDependInterruptSafeCall();

// Implementation
public:
    UInt32      tmIInterruptSafeCall(pTM_IRQ_SAFE_CALLBACK, void*);

// Attributes
private:
    //interrupt object
    PKINTERRUPT m_pIntObject;
    //status of object creation
    UInt32      m_dwObjectStatus;
};