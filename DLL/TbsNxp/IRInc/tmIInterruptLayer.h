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
//  20Apr04  CP      Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TM_INT_LAYER
#define TM_INT_LAYER

#include "tmTypes.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Describes the possible interrupt activation types.
//
//////////////////////////////////////////////////////////////////////////////
typedef enum
{
	FALLING_EDGE_ACTIVATED = 0,
	RISING_EDGE_ACTIVATED  = 1,
    BOTH_EDGES_ACTIAVTED

} tmInterruptActivateType;


class CInterruptCallback
{
//@access public members
public:
    //@access Public functions
    //@cmember UMCallback<nl>
    //Parameterlist:<nl>
    //PVOID pUMParam // <nl>
    //eEventType tEvent // <nl>
    virtual void InterruptCallback ( void* pUMParam ) = NULL;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:         
//
//  This layer provides access to the interrupts
//
//////////////////////////////////////////////////////////////////////////////
class tmIInterruptLayer
{
public:
    virtual ~tmIInterruptLayer(){};

     // Description:
	 //  The tmIEnableInterrupt function writes the given value to the selected GPIO 
     //  pin.
	 // Return Value:         
     //  Returns an UINT value. Possible values include the following:
	 // Return Value:         
     //  TM_OK                  The function executed successfully
	 //  TM_ERR_INVALID_COMMAND The interrupt is unknown.
	 //  TM_ERR_BAD_PARAMETER   The paramter is uncorrect
	 //  TM_ERR_NULL_PARAMETER  One parameter is NULL
    virtual UInt32 tmIEnableInterrupt(
                        //  [in] interupt (GPIO) number to be enabled
                        UInt8                   ucGPIOPin,
						//  [in] interupt activiation type (
						tmInterruptActivateType EventType,
						//  [in] pointer to the callback function
						CInterruptCallback*     pCallback
                        ) = 0;
    
     // Description:
	 //  The tmIDisableInterrupt function switches of the selected GPIO interrupt.
	 // Return Value:         
     //  Returns an UINT value. Possible values include the following:
	 // Return Value:         
     //  TM_OK                  The function executed successfully
	 //  TM_ERR_NOT_INITIALIZED The interface is not initialized.
	 //  TM_ERR_BAD_PARAMETER   The paramter is uncorrect
	 //  TM_ERR_INVALID_COMMAND The interrupt is unknown.
    virtual UInt32 tmIDisableInterrupt(
                        //  [in] interupt (GPIO) number to be disabled
                        UInt8                   ucGPIOPin,
                        tmInterruptActivateType EventType
						) = 0;
};

#endif
