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
//  12Oct05  CP      Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TM_OSDEPEND_REGISTRY_H
#define TM_OSDEPEND_REGISTRY_H

#include "tmTypes.h"
//#include "34AvStrm.h"
#include "tmIOSDependRegistry.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the registry access.
//  It is derived from the virtual class COSDependRegistry.
//
//////////////////////////////////////////////////////////////////////////////
class tmCOSDependRegistry : public tmIOSDependRegistry
{
// Construction
public:
    tmCOSDependRegistry( void* pRegistryHandle );

// Implementation
public:
    virtual ~tmCOSDependRegistry();

    UInt32 ReadRegistry(
        String pszValueName,
        String pszTargetBuffer,
        UInt32 dwTargetBufferLength,
        String pszRegistrySubkeyPath = NULL,
        String pszDefaultString      = NULL );

    UInt32 ReadRegistry(
        String  pszValueName,
        UInt32* pdwTargetBuffer,
        String  pszRegistrySubkeyPath = NULL,
        UInt32  dwDefaultValue        = 0 );

    UInt32 ReadRegistry(
        UInt32 dwTargetBufferLength,
        String pszTargetBuffer,
        String pszRegistryPath,
        String pszValueName,
        Bool   bUseRelUserKey = False );

    UInt32 ReadRegistry(
        String  pszValueName,
        String  pszRegistryPath,
        UInt32* pdwTargetBuffer,
        Bool    bUseRelUserKey = False );

    UInt32 WriteRegistry(
        String pszValueName,
        String pszString,
        String pszRegistrySubkeyPath = NULL );

    UInt32 WriteRegistry(
        String pszValueName,
        UInt32 dwValue,
        String pszRegistrySubkeyPath = NULL );

// Attributes
private:
    void* m_pvHandle;    
};

#endif
