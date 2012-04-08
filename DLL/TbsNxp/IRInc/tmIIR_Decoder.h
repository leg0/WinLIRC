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

#ifndef IN
#define IN
#endif

#include "tmTypes.h"
#include "tmIR_DecoderTypes.h"
#include "tmIGpioLayer.h"
#include "tmIInterruptLayer.h"
#include "tmIOSDependFactory.h"

typedef struct
{
    tmIrDecoderId           tDecoderId;
    UInt32                  dwGpioNr;
    tmInterruptActivateType tInterruptType;
    Bool                    bInterruptDecoding;
//Lawrence	tmIrDecodingMode		tDecodingMode;
} tmIrHwInfo_t, *ptmIrHwInfo_t;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This class provides a generic interface for accessing the IR decoder.
//  The class is virtual, it only defines a set of methods. The implementation
//  of these depends on the real IR decoder that should be supported.
//
//////////////////////////////////////////////////////////////////////////////
class tmIIrDecoder
{
public:
    tmIIrDecoder( CIR_DecoderCallback* pIR_DecoderCallback,
		          tmIGPIOLayer*        ptGpioLayer,
			  	  tmIInterruptLayer*   ptInterruptLayer,
                  tmIOSDependFactory*  ptFactory,
                  ptmIrHwInfo_t        ptHwInfo );

    virtual ~tmIIrDecoder();

private:
    tmComcreteIrDecoder* m_pDecoder;
};