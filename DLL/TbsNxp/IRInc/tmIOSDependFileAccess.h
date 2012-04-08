//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2003
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

#ifndef TM_OSDEPEND_FILE_ACCESS_H
#define TM_OSDEPEND_FILE_ACCESS_H

#include "tmTypes.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class provides access to a specified file in the system.
//
//////////////////////////////////////////////////////////////////////////////
class tmIOSDependFileAccess
{
public:
    //  the destructor needs to be virtual, because otherwise the destructor
    //  of the deriven class never gets called!
    virtual ~tmIOSDependFileAccess(){};
    //  This method reads specified amount of bytes from a specified file.
    //  The file location is OS dependant
    // Parameters:
    //  dwNumBytes - requested number of bytes to read. Value must be between
    //               1 and 500k.
    virtual UInt32 tmReadBytes(String pszFileName,
                               UInt8* pBytesArray,
                               UInt32 dwNumBytes) = 0;
};

#endif

