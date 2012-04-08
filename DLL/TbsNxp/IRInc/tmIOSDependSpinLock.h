//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
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
//  18Oct99  FH      Created
//  06MAY02  AS      Added comments
//  27Jun02  ML      Code review + PC-Lint
//  05JUL02  AW      rework after code review
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TM_OSDEPEND_SPIN_LOCK_H
#define TM_OSDEPEND_SPIN_LOCK_H

#include "tmTypes.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the spin lock.
//
//////////////////////////////////////////////////////////////////////////////
class tmIOSDependSpinLock
{
// Implementation
public:
    virtual      ~tmIOSDependSpinLock() {}
    virtual void tmAcquireSpinLock() = 0;
    virtual void tmReleaseSpinLock() = 0;

};

#endif

