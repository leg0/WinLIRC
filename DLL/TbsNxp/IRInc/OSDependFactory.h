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

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.

#include "tmIOSDependFactory.h"


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the OS Factory. It is derived
//  from the virtual class tmIOSDependObjectFactory.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependFactory : public tmIOSDependFactory
{
public:
	//  creates delayexecution object
	// Return value:
	//  pointer to created object
    tmIOSDependDelayExecution* 
		tmCreateDelayExecution();
	//  creates file access object
	// Return value:
	//  pointer to created object
    tmIOSDependFileAccess* 
		tmCreateFileAccess();
	// creates a timer object
	// Return value:
	//  pointer to created object
	tmIOSDependTimeOut*
		tmCreateTimer();
	// creates a spin lock object
	// Return value:
	//  pointer to created object
	tmIOSDependSpinLock*
		tmCreateSpinLock();
	// creates a thread object
	// Return value:
	//  pointer to created object
	tmIOSDependThread*
		tmCreateThread();
    // creates an event object
	// Return value:
	//  pointer to created object
	tmIOSDependEvent*
		tmCreateEvent();
    // creates an page table object
    // Return value:
    //  pointer to created object
    tmIOSDependPageTable*
        tmCreatePageTable();
    // creates a registry object
    // Return value:
    //  pointer to created object
    tmIOSDependRegistry*
        tmCreateRegistry( void* );

};