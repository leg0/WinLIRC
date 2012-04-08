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

#ifndef TM_OSDEPEND_FACTORY_H
#define TM_OSDEPEND_FACTORY_H

#ifndef IN
#define IN
#endif


#include "tmIOSDependDelayExecution.h"
#include "tmIOSDependFileAccess.h"
#include "tmIOSDependTimeOut.h"
#include "tmIOSDependSpinLock.h"
#include "tmIOSDependThread.h"
#include "tmIOSDependEvent.h"
#include "tmIOSDependPageTable.h"
#include "tmIOSDependRegistry.h"

#define DEBUGLVL_ERROR 0


//////////////////////////////////////////////////////////////////////////////
//
// Description: 
//
//  This class allows to create instances of tmIOSDepend classes 
//  without knowing how the class is implemented in the driver.
//
//////////////////////////////////////////////////////////////////////////////
class tmIOSDependFactory
{
public:
    //  creates delayexecution object
    // Return value:
    //  pointer to created object
    virtual tmIOSDependDelayExecution* 
        tmCreateDelayExecution() = 0;
    //  creates file access object
    // Return value:
    //  pointer to created object
    virtual tmIOSDependFileAccess* 
        tmCreateFileAccess() = 0;
    // creates a timer object
    // Return value:
    //  pointer to created object
    virtual tmIOSDependTimeOut*
        tmCreateTimer() = 0;
    // creates a spin lock object
    // Return value:
    //  pointer to created object
    virtual tmIOSDependSpinLock*
        tmCreateSpinLock() = 0;
    // creates a thread object
    // Return value:
    //  pointer to created object
    virtual tmIOSDependThread*
        tmCreateThread() = 0;
    // creates an event object
    // Return value:
    //  pointer to created object
    virtual tmIOSDependEvent*
        tmCreateEvent() = 0;
    // creates an page table object
    // Return value:
    //  pointer to created object
    virtual tmIOSDependPageTable*
        tmCreatePageTable() = 0;
    // creates a registry object
    // Return value:
    //  pointer to created object
    virtual tmIOSDependRegistry*
        tmCreateRegistry( void* ) = 0;

};

#if _DEBUG

void DbgFunc(UInt8  ucDebugLevel, 
             Char*  format, 
             ...);

void DbgFuncL(UInt8  ucDebugLevel);

void DbgFuncP(Char*  format, 
             ...);

void DbgFuncErrorInfo
( 
    UInt16 nFileLine,    // Integer value contains the file line info
    Char*  pcFunction    // Pointer to Null terminated String (contains file info)
);
                
//
//  Defines a Debug print macro. BUGBUG AW: needed for downcompatibility
//  Eg:     tmDBGPRINT((1,"CVampExampleModule - Constructor %d", 61));
// Parameters:
//  args - arguments for a debug call

#define tmDBGPRINT(args) DbgFunc args; 

//
//  Defines a Debug print macro. 
//  Eg:     tmDBGPRINT(1,("CVampExampleModule - Constructor %d", 61));
// Parameters:
//  dbglvl
//  args - arguments for a debug call

#define tmDBGPRINTEx(lvl, strings) \
DbgFuncL(lvl);    \
    if ( 0 == lvl ) \
{                 \
        DbgFuncErrorInfo( (UInt16)__LINE__, __FILE__ ); \
    }                \
DbgFuncP strings ;


#else // DBG


void DbgFunc(UInt8  ucDebugLevel, 
             Char*  format, 
             ...);

void DbgFuncL(UInt8  ucDebugLevel);

void DbgFuncP(Char*  format, 
             ...);

void DbgFuncErrorInfo
( 
    UInt16 nFileLine,    // Integer value contains the file line info
    Char*  pcFunction    // Pointer to Null terminated String (contains file info)
);
                
//
//  Defines a Debug print macro. BUGBUG AW: needed for downcompatibility
//  Eg:     tmDBGPRINT((1,"CVampExampleModule - Constructor %d", 61));
// Parameters:
//  args - arguments for a debug call

#define tmDBGPRINT(args) DbgFunc args; 

//
//  Defines a Debug print macro. 
//  Eg:     tmDBGPRINT(1,("CVampExampleModule - Constructor %d", 61));
// Parameters:
//  dbglvl
//  args - arguments for a debug call

#define tmDBGPRINTEx(lvl, strings) \
    DbgFuncL(lvl);                 \
    if ( 0 == lvl )                \
    {                              \
        DbgFuncErrorInfo( (UInt16)__LINE__, __FILE__ ); \
    }                              \
    DbgFuncP strings ;


//#define tmDBGPRINTEx(lvl, strings)

#endif

#endif

