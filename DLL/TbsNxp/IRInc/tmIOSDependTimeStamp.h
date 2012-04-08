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

#ifndef TMIOSDEPENDTIMESTAMP_H
// Description:
//  This define avoids multiple including of the header content.
#define TMIOSDEPENDTIMESTAMP_H

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the time stamp.
//
//////////////////////////////////////////////////////////////////////////////
class tmIOSDependTimeStamp
{
// Construction
public:
    //  we need this destructor implementation and it has to be virtual, 
    //  because otherwise the destructor of the deriven class never gets 
    //  called!
      virtual ~tmIOSDependTimeStamp(){};

// Implementation
public:
      //  this function returns the current stream time in a 64 bit value,
      //  resolution is 100ns
      // Parameters:
      //  pqwTimeStamp   - current stream time
      // Return Value:
      //  TM_ERR_NOT_SUPPORTERD   the function is not implemented
      //  TM_OK                   the function completed successfully
      virtual UInt32 tmIGetTimeStamp(UInt64* pqwTimeStamp) = 0;

// Attributes
private:

};
#endif

