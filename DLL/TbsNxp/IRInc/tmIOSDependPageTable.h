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
//  06Oct05  PA      Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TM_OSDEPEND_PAGETABLE_H
#define TM_OSDEPEND_PAGETABLE_H

#include "tmTypes.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the interface of an OS depend page table.
//
//////////////////////////////////////////////////////////////////////////////
class tmIOSDependPageTable
{
// Implementation
public:
    //  the destructor needs to be virtual, because otherwise the destructor
    //  of the deriven class never gets called!
    virtual      ~tmIOSDependPageTable() {}
    //
    // Description:
    //  The tmGetPhysicalAddress method returns a pointer to the physical 
    //  aligned page table address.
    // Return Value:
    //  TM_ERR_NOT_INITIALIZED  No valid physical address available.
    //  TM_ERR_BAD_PARAMETER    A bad input parameter was detected.
    //  TM_OK                   The event was cleared successfully.
    //
    virtual UInt32 tmGetPhysicalAddress(UInt64* pqwPhysicalAddress ) = 0;
    //
    // Description:
    //  The tmGetVirtulaAddress method returns a pointer to the linear
    //  aligned page table address
    // Return Value:
    //  TM_ERR_NOT_INITIALIZED  No valid linear address available.
    //  TM_ERR_BAD_PARAMETER    A bad input parameter was detected.
    //  TM_OK                   The event state was returned successfully.
    //
    virtual UInt32 tmGetVirtualAddress( void** ppvVirtualAddress ) = 0;
};

#endif

