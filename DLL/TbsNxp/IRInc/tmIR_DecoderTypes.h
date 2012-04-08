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
//  26Jul05  CP      Created
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "tmTypes.h"
#include "tmIOSDependFactory.h"


typedef enum
{
	IR_DECODER_ID_RC5          = 0x00,
	IR_DECODER_ID_RC6          = 0x01,
	IR_DECODER_ID_NEC_32       = 0x02,
	IR_DECODER_ID_NEC_40       = 0x03,

    MAX_IR_DECODER             = 0x04

} tmIrDecoderId;


class CIR_DecoderCallback  
{
// @access Public Members
public:
	// @cmember virtual function to be implemented in the application level
	// to form the call back from CRC5_Interface class to the application
	// level
	virtual void IR_DecoderCallback( tmIrDecoderId Id, void* ptData ) = NULL;
};

// error code for decoding procedure
typedef enum
{
	TM_IR_DECODER_INVALID_START          = 0x01,
	TM_IR_DECODER_INVALID_BIT_DURATION,
	TM_IR_DECODER_INVALID_BIT_TEMPLATE,
	TM_IR_DECODER_SAMPLING_FREQUENCY_TOO_LOW,
	TM_IR_DECODER_INSUFFICIENT_SAMPLES,
	TM_IR_DECODER_TOO_MANY_SAMPLES,
    TM_IR_DECODER_INV_DATA_NOT_MATCH,
    TM_IR_DECODER_PENDING,
	TM_IR_DECODER_OK
} tmIrDecoderErrorCode_t;

// structure defined to store a pressed key inforation
typedef struct
{
   UInt8                  Bits[32];
   UInt16                 Address;
   UInt8                  Data;
   Bool                   Repeat;

} tmIrDecoderNec32Data_t;

// structure defined to store a pressed key inforation
typedef struct
{
   UInt8                  Bits[32];
   UInt32                 Address;
   UInt8                  Data;
   Bool                   Repeat;

} tmIrDecoderNec40Data_t;

// structure defined to store a pressed key inforation
typedef struct
{
   UInt32                 Header;
   UInt16                 Control;
   UInt16                 Information;

} tmIrDecoderRC6Data_t;

// structure defined to store a pressed key inforation
typedef struct
{
   UInt8                  Command;
   UInt8                  Group;
   UInt8                  Toggle;

} tmIrDecoderRC5Data_t;

// structure defined for RLC decoding
typedef struct
{
    Int16*                pnSample;
    UInt16                nNrOfSamples;

} tmIrDecoderRLC_t;


class tmComcreteIrDecoder
{
public:
    tmComcreteIrDecoder(){};

    virtual ~tmComcreteIrDecoder(){};
};

