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
#include "34AVStrm.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class handles memory operations like locking/unlocking pages and 
//  getting page tables for the MMU.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependMemory
{
// Attribute
private:
    ULONG   m_cBytes;
    ULONG*  m_pulVirtualPTAddress;
    PVOID   m_pMDL;
    DWORD   m_dwObjectStatus;

public:
// Construction
    COSDependMemory();
    virtual  ~COSDependMemory();

// Implementation    
    PVOID    LockPages( PVOID  pVirtualAddress, 
                        size_t  NumberOfBytes );

    PVOID    LockUserPages( PVOID  pVirtualAddress,
                            size_t NumberOfBytes );
    
    void     UnlockUserPages( PVOID pHandle );

    UInt32   GetPageTable( PVOID    pHandle, 
                           PDWORD*  pdwPageEntries, 
                           DWORD*   pdwNumberOfEntries );

    UInt32 GetPageTableFromMappings(
        PVOID pMemList,
        PDWORD *ppdwPageEntries,     
        DWORD *pdwNumberOfEntries);

    //PVOID    GetSystemAddress( PVOID pHandle );
    void     UnlockPages ( PVOID pHandle );
    DWORD    GetObjectStatus();
};




