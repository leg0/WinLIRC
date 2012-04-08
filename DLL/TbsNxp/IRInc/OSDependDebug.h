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
//#include "tmIOSDependEvent.h"
//#include "34AVStrm.h"

//////////////////////////////////////////////////////////////////////////////
//
// the maximum length for a debug string
//
//////////////////////////////////////////////////////////////////////////////
#define MAX_DEBUGSTRING_LENGTH  256

//////////////////////////////////////////////////////////////////////////////
//
// the maximum length for a debug prolog string
//
//////////////////////////////////////////////////////////////////////////////
#define MAX_DEBUGPROLOG_LENGTH  256

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class is for runtime debugging. The COSDependDebug methods should be 
//  accessed only via macros.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependDebug  
{
// Construction
public:
    COSDependDebug();
    virtual ~COSDependDebug();

// Implementation
public:
    void Debug_Print_Prolog(
            PCHAR pszFile, 
            int   nLine );

    void Debug_Print(
            PCHAR pszDebugString, 
            ... );

    void Debug_Print(
            BYTE  ucDebugLevel, 
            PCHAR format, 
            ... );

    void Debug_Break( BYTE ucDebugBreakLevel );

    void SetCurrentClassID( DWORD dwClassID );

// Attribute
private:
//    BYTE   m_ucDebugLevel[LAST_DEBUG_CLASS_ID + 1];     //debug level
    CHAR   m_pszDebugProlog[MAX_DEBUGPROLOG_LENGTH];    //debug prolog
    CHAR   m_pszDebugString[MAX_DEBUGSTRING_LENGTH];    //debug string
//    DWORD  m_dwClassID;                     //class id
//    COSDependRegistry*  m_pRegistryAccess;  //registry access instance
};

extern COSDependDebug* g_pDebugObject;



