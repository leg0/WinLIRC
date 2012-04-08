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
#include "stdafx.h"
#include "tmTypes.h"

//#include "OSDepend.h"
#ifndef IN
#define IN
#endif


#include "OSDependFactory.h"
#include "OSDependDelayExecution.h"
#include "OSDependFileAccess.h"
#include "OSDependTimeOut.h"
#include "OSDependSpinLock.h"
#include "OSDependThread.h"
#include "OSDependEvent.h"
#include "OSDependPageTable.h"
#include "OSDependRegistry.h"
#include "stdio.h"
//#include "KSDebug.h"

//#if _DEBUG


//static debug level
UInt8 g_ucDebugLevel = 0;
/*
//helper function to send debug messages
void DbgFunc
(
    UInt8  ucDebugLevel,      //Debug Level that has to be lower or equal to the
                            //current class debug level to print out the debug
                            //message
    Char* pszDebugString,   //Pointer to NULL terminated String (contains debug
                            //message in a format that is similar to ANSI
                            //printf)
    ...                     //variable argument list
)
{/*
    //declare variable to be an argument list pointer
    va_list ap;
    CHAR pszTempDebugString[256];
    switch(ucDebugLevel)
    {
    case 0:
        //declare variable to be an argument list pointer
        //set pointer to the beginning of the argument list.
        va_start(ap, pszDebugString);

        //print the variable arguments into the debug string
        vsprintf(pszTempDebugString, pszDebugString, ap);
        //
      //  _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, STR_MODULENAME );
       // _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, pszTempDebugString);
      //  _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, "\n");
        //DbgBreakPoint();
        _DbgPrintF(DEBUGLVL_ERROR,(pszTempDebugString));
        //reset the argument pointer
        va_end(ap);
        break;
    case 1:
        //set pointer to the beginning of the argument list.
        va_start(ap, pszDebugString);

        //print the variable arguments into the debug string
        vsprintf(pszTempDebugString, pszDebugString, ap);
        //
       // _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, STR_MODULENAME );
      //  _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, pszTempDebugString);
      //  _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, "\n");
        _DbgPrintF(DEBUGLVL_VERBOSE,(pszTempDebugString));
        //reset the argument pointer
        va_end(ap);
        break;
    default:
        //set pointer to the beginning of the argument list.
        va_start(ap, pszDebugString);

        //print the variable arguments into the debug string
        vsprintf(pszTempDebugString, pszDebugString, ap);
        //
   //     _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, STR_MODULENAME );
   //     _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, pszTempDebugString);
  //      _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, "\n");
        _DbgPrintF(DEBUGLVL_BLAB,(pszTempDebugString));
        //reset the argument pointer
        va_end(ap);
        break;
    }
    char DebugString[200];

    sprintf( DebugString, "%s\n", pszDebugString );
}*/

//helper function to send debug messages
void DbgFuncP
( 
    Char* pszDebugString,   //Pointer to NULL terminated String (contains debug
                            //message in a format that is similar to ANSI
                            //printf)
    ...                     //variable argument list
)
{
    //declare variable to be an argument list pointer
    va_list ap;
    CHAR pszTempDebugString[256];
    switch(g_ucDebugLevel)
    {
    case 0:
        //declare variable to be an argument list pointer
        //set pointer to the beginning of the argument list.
        va_start(ap, pszDebugString);

        //print the variable arguments into the debug string
        vsprintf_s(pszTempDebugString, pszDebugString, ap);
        //
   //     _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, STR_MODULENAME );
  //      _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, pszTempDebugString);
  //      _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, "\n");
        //DbgBreakPoint();
//         OutputDebugString( L(pszTempDebugString) );
//         OutputDebugString( L"\n" );
        //reset the argument pointer
        va_end(ap);
        break;
    case 1:
        //set pointer to the beginning of the argument list.
        va_start(ap, pszDebugString);

        //print the variable arguments into the debug string
        vsprintf_s(pszTempDebugString, pszDebugString, ap);
        //
   //     _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, STR_MODULENAME );
   //     _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, pszTempDebugString);
   //     _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, "\n");
//         OutputDebugString( L(pszTempDebugString) );
//         OutputDebugString( L("\n") );
        //reset the argument pointer
        va_end(ap);
        break;
    default:
        //set pointer to the beginning of the argument list.
        va_start(ap, pszDebugString);

        //print the variable arguments into the debug string
        vsprintf_s(pszTempDebugString, pszDebugString, ap);
        //
     //   _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, STR_MODULENAME );
     //   _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, pszTempDebugString);
     //   _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, "\n");
//         OutputDebugString( L(pszTempDebugString) );
//         OutputDebugString( L"\n" );
        //reset the argument pointer
        va_end(ap);
        break;
    }    
}

//helper function to send detailed error messages
void DbgFuncErrorInfo
( 
    UInt16 nFileLine,    // Integer value contains the file line info
    Char*  pcFunction    // Pointer to NULL terminated String (contains file info)
)
{
    char pszTempDebugString[500];

    if ( NULL == pcFunction )
    {
        return;
    }

    //print the variable arguments into the debug string
    sprintf_s( pszTempDebugString, "Error: In function: %s (Line %d)\n", pcFunction, nFileLine );

 //   _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, STR_MODULENAME );
  //  _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, pszTempDebugString);
 //   _DbgPrintFEx(DPFLTR_IHVSTREAMING_ID, DPFLTR_ERROR_LEVEL, "\n");
    //_DbgPrintF(DEBUGLVL_BLAB,(pszTempDebugString));

//    OutputDebugString( L(pszTempDebugString) );
}

//helper function to set the debug level
void DbgFuncL
(
    UInt8  ucDebugLevel     //Debug Level that has to be lower or equal to the
                            //current class debug level to print out the debug
                            //message
)
{
    g_ucDebugLevel = ucDebugLevel;
}

//#endif

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of tmIOSDependDelayExecution. We do not allow
//  paged pool allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the tmIOSDependDelayExecution object
//
//////////////////////////////////////////////////////////////////////////////
tmIOSDependDelayExecution* COSDependFactory::tmCreateDelayExecution()
{
    //_DbgPrintF(DEBUGLVL_BLAB,("Info: IOSCreateDelayExecution called"));
    // return allocated delay execution object, if the memory is not
    // available NULL is returned by the new operator
    return new tmCOSDependDelayExecution();
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of tmIOSDependFileAccess. We do not allow
//  paged pool allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the tmIOSDependFileAccess object
//
//////////////////////////////////////////////////////////////////////////////
tmIOSDependFileAccess* COSDependFactory::tmCreateFileAccess()
{
    //_DbgPrintF(DEBUGLVL_BLAB,("Info: CreateFileAccess called"));
    // return allocated file access object, if the memory is not
    // available NULL is returned by the new operator
    return 0;//new tmCOSDependFileAccess();
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of tmIOSDependFileAccess. We do not allow
//  paged pool allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the tmIOSDependFileAccess object
//
//////////////////////////////////////////////////////////////////////////////
tmIOSDependTimeOut* COSDependFactory::tmCreateTimer()
{
    //_DbgPrintF(DEBUGLVL_BLAB,("Info: CreateTimer called"));
    // return allocated timer object, if the memory is not
    // available NULL is returned by the new operator
    return new tmCOSDependTimeOut();
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of tmIOSDependSpinLock. We do not allow
//  paged pool allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the tmIOSDependSpinLock object
//
//////////////////////////////////////////////////////////////////////////////
tmIOSDependSpinLock* COSDependFactory::tmCreateSpinLock()
{
    //_DbgPrintF(DEBUGLVL_BLAB,("Info: tmCreateSpinLock called"));
    // return allocated spin lock object, if the memory is not
    // available NULL is returned by the new operator
    return 0;//new tmCOSDependSpinLock();
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of tmIOSDependThread. We do not allow
//  paged pool allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the tmIOSDependThread object
//
//////////////////////////////////////////////////////////////////////////////
tmIOSDependThread* COSDependFactory::tmCreateThread()
{
    //_DbgPrintF(DEBUGLVL_BLAB,("Info: tmCreateThread called"));
    // return allocated thread object, if the memory is not
    // available NULL is returned by the new operator
    return 0;//new tmCOSDependThread();
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of tmIOSDependEvemt. We do not allow
//  paged pool allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the tmIOSDependEvent object
//
//////////////////////////////////////////////////////////////////////////////
tmIOSDependEvent* COSDependFactory::tmCreateEvent()
{
    //_DbgPrintF(DEBUGLVL_BLAB,("Info: tmCreateEvent called"));
    // return allocated event object, if the memory is not
    // available NULL is returned by the new operator
    return 0;//new tmCOSDependEvent();
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of tmIOSDependEvemt. We do not allow
//  paged pool allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the tmIOSDependEvent object
//
//////////////////////////////////////////////////////////////////////////////
tmIOSDependPageTable* COSDependFactory::tmCreatePageTable()
{
    //_DbgPrintF(DEBUGLVL_BLAB,("Info: tmCreatePageTable called"));
    // return allocated page table object, if the memory is not
    // available NULL is returned by the new operator
    return 0;//new tmCOSDependPageTable();
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of tmIOSDependEvemt. We do not allow
//  paged pool allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the tmIOSDependEvent object
//
//////////////////////////////////////////////////////////////////////////////
tmIOSDependRegistry* COSDependFactory::tmCreateRegistry( void* Handle )
{
    //_DbgPrintF(DEBUGLVL_BLAB,("Info: tmCreatePageTable called"));
    // return allocated page table object, if the memory is not
    // available NULL is returned by the new operator
    return 0;//new tmCOSDependRegistry( Handle );
};
