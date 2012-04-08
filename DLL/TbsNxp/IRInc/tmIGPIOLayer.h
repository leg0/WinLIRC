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
//  24Jan03  AW      Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TM_GPIO_LAYER
#define TM_GPIO_LAYER

#include "tmTypes.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:         
//
//  This layer provides access to the GPIO bus
//
//////////////////////////////////////////////////////////////////////////////
class tmIGPIOLayer
{
public:
    virtual ~tmIGPIOLayer(){};

     // Description:
	 //  The tmISetPin function writes the given value to the selected GPIO 
     //  pin.
	 // Return Value:         
     //  Returns an UINT value. Possible values include the following:
	 // Return Value:         
     //  TM_OK                  The function executed successfully
	 //  TM_ERR_NOT_INITIALIZED The interface is not initialized.
	 //  TM_ERR_INVALID_COMMAND The GPIO write sequence failed.
    virtual UInt32 tmISetPin(
                        //  [in] number of pin to be set
                        UInt8 ucGPIOPin,  
                         // [in] value to be set (must be 1 or 0)
                        UInt8 ucValue) = 0;
    
     // Description:
	 //  The tmReadSequence function reads the given value from the selected 
     //  GPIO pin.
	 // Return Value:         
     //  Returns an UINT value. Possible values include the following:
	 // Return Value:         
     //  TM_OK                  The function executed successfully
	 //  TM_ERR_NOT_INITIALIZED The interface is not initialized.
	 //  TM_ERR_INVALID_COMMAND The GPIO read sequence failed.
    virtual UInt32 tmIGetPin(
                        //  [in] number of pin to get the value from
                        UInt8 ucGPIOPin,  
                         // [out] pointer to value to be read (will be 1 or 0)
                        UInt8* pucValue) = 0;
};

#endif

