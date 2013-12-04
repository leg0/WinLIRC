/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 2011 Artem Golubev
 */

#include "Globals.h"
#include "SendReceiveData.h"
#include "../Common/Linux.h"

DWORD WINAPI BeholdRC(void *recieveClass)
{
   ((SendReceiveData*)recieveClass)->threadProc();
   return 0;
}

SendReceiveData::SendReceiveData()
{
	threadHandle = NULL;
	exitEvent = NULL;
}


SendReceiveData::~SendReceiveData()
{
}


BOOL SendReceiveData::init()
{
	
	threadHandle = NULL;
	exitEvent = NULL;

	gettimeofday(&end,NULL);	// initialise

	if( !BTV_GetIStatus() ) {
		MessageBox( 0, _T( "Library not found" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
		return false;
	}

   /*
   int status = BTV_GetIStatus();
   switch( status ) {
      case 0: // Library not found.
         MessageBox( 0, _T( "Library not found" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
         break;
      case 1:	// WDM device not selected.
         MessageBox( 0, _T( "WDM device not selected" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
         break;
      case 2:	// OK.
         MessageBox( 0, _T( "OK" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
         break;

      default:
         MessageBox( 0, _T( "default" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
         break;
   }
   */

   if( BTV_SelectCard() ) {
      threadHandle = CreateThread( NULL, 0, BeholdRC, (void *)this, 0, NULL );
      if( threadHandle ) {
         return true;
      }
   }

   MessageBox( 0, _T( "Error selecting card" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
   return false;
}

void SendReceiveData::deinit()
{
   killThread();
}


void SendReceiveData::threadProc()
{
	exitEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	if( !exitEvent )
		return;

	DWORD result = 0;

	while( TRUE ) {

		getCode();

		result = WaitForSingleObject( exitEvent, 30 );
		if( result == (WAIT_OBJECT_0) )
		{
			// leaving thread
			break;
		}
	}

	if( exitEvent ) {
		CloseHandle( exitEvent );
		exitEvent = NULL;
	}
}

void SendReceiveData::killThread()
{
	//
	// need to kill thread here
	//
	if( exitEvent ) {
		SetEvent( exitEvent );
	}

	if( threadHandle != NULL )
   {
		DWORD result = 0;

		if( GetExitCodeThread( threadHandle, &result ) == 0 ) 
		{
			CloseHandle( threadHandle );
			threadHandle = NULL;
			return;
		}

		if( result==STILL_ACTIVE )
		{
			WaitForSingleObject( threadHandle, INFINITE );
		}

		CloseHandle( threadHandle );
		threadHandle = NULL;
	}
}

bool SendReceiveData::waitTillDataIsReady( int maxUSecs )
{
	HANDLE events[2] = { dataReadyEvent, threadExitEvent };
	int evt;
	if( threadExitEvent == NULL )
      evt = 1;
	else
      evt = 2;

	if(!dataReady())
	{
		ResetEvent( dataReadyEvent );

		int res;
		if( maxUSecs )
			res = WaitForMultipleObjects( evt, events, FALSE, ( maxUSecs + 500 ) / 1000 );
		else
			res = WaitForMultipleObjects( evt, events, FALSE, INFINITE );
		if( res == (WAIT_OBJECT_0+1) )
		{
			return false;
		}
	}

	return true;
}

void SendReceiveData::getCode()
{
	//========================
	ir_code tempCode;
	struct mytimeval tempStart;
	struct mytimeval tempEnd;
	struct mytimeval tempLast;
	//========================

	//
	// get code see if its valid because we are polling
	// 99% of the time it will just return zero
	//

	gettimeofday(&tempStart,NULL);
	tempLast = end;

	tempCode = BTV_GetRCCodeEx();

	gettimeofday(&tempEnd,NULL);

	if(tempCode) {

		EnterCriticalSection(&criticalSection);
			start	= tempStart;
			last	= tempLast;
			end		= tempEnd;
			irCode	= tempCode;
		LeaveCriticalSection(&criticalSection);

		SetEvent( dataReadyEvent );
	}	
}


int	SendReceiveData::dataReady() {

	//=========
	int result;
	//=========

	result = WaitForSingleObject(dataReadyEvent,0);

	if(result==WAIT_OBJECT_0) return 1;
	
	return 0;
}
