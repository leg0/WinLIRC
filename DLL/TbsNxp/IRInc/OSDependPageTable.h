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

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.

#include "tmIOSDependPageTable.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the time out. It is derived
//  from the virtual class tmCOSDependPageTable.
//
//////////////////////////////////////////////////////////////////////////////

class tmCOSDependPageTable : public tmIOSDependPageTable
{
public:
    //  the destructor needs to be virtual, because otherwise the destructor
    //  of the deriven class never gets called!
            // create empty page table
            tmCOSDependPageTable( );
            // create empty page table and then translate buffer
            //tmCOSDependPageTable( void* pvBuffer, UInt32 dwSize );
    virtual ~tmCOSDependPageTable();
    //
    // Description:
    //  The tmGetPhysicalAddress method returns a pointer to the physical 
    //  aligned page table address.
    // Return Value:
    //  TM_ERR_NOT_INITIALIZED  No valid physical address available.
    //  TM_ERR_BAD_PARAMETER    A bad input parameter was detected.
    //  TM_OK                   The event was cleared successfully.
    //
    UInt32 tmGetPhysicalAddress(UInt64* pqwPhysicalAddress );
    //
    // Description:
    //  The tmGetVirtulaAddress method returns a pointer to the linear
    //  aligned page table address
    // Return Value:
    //  TM_ERR_NOT_INITIALIZED  No valid linear address available.
    //  TM_ERR_BAD_PARAMETER    A bad input parameter was detected.
    //  TM_OK                   The event state was returned successfully.
    //
    UInt32 tmGetVirtualAddress( void** ppvVirtualAddress );
    
private:
    UInt32   m_tmStatus;                   // object status
    void*    m_pvVirtualAddress;           // virtual address of the page table
    void*    m_pvUnalignedVirtualAddress;  // unaligned virtual address
    UInt64   m_qwPhysicalAddress;          // physical address of the page table
};
