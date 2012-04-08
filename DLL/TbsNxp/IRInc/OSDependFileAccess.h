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
//  26Feb03  AW      Created
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "tmIOSDependFileAccess.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class provides file access.
//
//////////////////////////////////////////////////////////////////////////////
class tmCOSDependFileAccess : public tmIOSDependFileAccess
{
public:
    tmCOSDependFileAccess();
    ~tmCOSDependFileAccess();

    UInt32 tmReadBytes(String pszFileName,
                       UInt8* pBytesArray,
                       UInt32 dwNumBytes);

private:
};