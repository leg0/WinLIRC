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
//  10Jul04  AW      Created
//
//////////////////////////////////////////////////////////////////////////////

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.

#include "tmIOSDependTimeStamp.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the time stamp. It is derived
//  from the virtual class tmIOSDependTimeStamp.
//
//////////////////////////////////////////////////////////////////////////////
class tmCOSDependTimeStamp : public tmIOSDependTimeStamp
{
public:
    tmCOSDependTimeStamp(PKSPIN pKSPin);
    //  the destructor needs to be virtual, because otherwise the destructor
    //  of the deriven class never gets called!
    virtual ~tmCOSDependTimeStamp();
    //  this function returns the current stream time in a 64 bit value, 
    //  resolution is 100ns
	// Parameters:
	//  pqwTimeStamp   - current stream time
	// Return Value:
	//  TM_ERR_NOT_SUPPORTERD   the function is not implemented
	//  TM_OK                   the function completed successfully
    UInt32 tmIGetTimeStamp(UInt64* pqwTimeStamp);
private:
    //status of object creation
    UInt32             m_dwObjectStatus;
    //iks clock instance
    PIKSREFERENCECLOCK m_pIKsClock;
};
