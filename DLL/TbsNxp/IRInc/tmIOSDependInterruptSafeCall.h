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

#ifndef TMI_OSDEPEND_INTERRUPT_SAFECALL_H
// Description:
//  This define avoids multiple including of the header content.
#define TMI_OSDEPEND_INTERRUPT_SAFECALL_H

#include "tmICallback.h"

// Description:
//  This is the prototype for the callback function
typedef UInt8 (*pTM_IRQ_SAFE_CALLBACK) (IN void* SynchronizeContext);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the interface of an interrupt safe call.
//
//////////////////////////////////////////////////////////////////////////////
class tmIOSDependInterruptSafeCall
{
// Construction
public:
	//  we need this destructor implementation and it has to be virtual, 
    //  because otherwise the destructor of the derived class never gets 
    //  called!
    virtual     ~tmIOSDependInterruptSafeCall() {};

// Implementation
public:
    // Description:
    //  The tmIInterruptSafeCall function calls the given callback in an
    //  interrupt safe context.
    // Return Value:
    //  TM_ERR_NOT_SUPPORTED   The function is not implemented.
	virtual UInt32      tmIInterruptSafeCall(pTM_IRQ_SAFE_CALLBACK pCallback,
                                             void* pContext) = 0;

// Attributes
private:

};
#endif // __TMI_OSDEPEND_INTERRUPT_SAFECALL_H
