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
#include "../Common/Win32Helpers.h"

DWORD WINAPI BeholdRC(void *recieveClass)
{
   ((SendReceiveData*)recieveClass)->threadProc();
   return 0;
}

SendReceiveData::SendReceiveData()
{
	threadHandle = nullptr;
	exitEvent = nullptr;
}


SendReceiveData::~SendReceiveData()
{
}


BOOL SendReceiveData::init()
{
	
	threadHandle = nullptr;
	exitEvent = nullptr;

	end = std::chrono::steady_clock::now();

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
      threadHandle = CreateThread( nullptr, 0, BeholdRC, (void *)this, 0, nullptr );
      if( threadHandle ) {
         return true;
      }
   }

   MessageBox( 0, _T( "Error selecting card" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
   return false;
}

void SendReceiveData::deinit()
{
	KillThread(exitEvent,threadHandle);
}

void SendReceiveData::threadProc()
{
	exitEvent = CreateEvent( nullptr, TRUE, FALSE, nullptr );

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

	SAFE_CLOSE_HANDLE(exitEvent);
}

bool SendReceiveData::waitTillDataIsReady( int maxUSecs )
{
	HANDLE events[2] = { dataReadyEvent, threadExitEvent };
	int evt;
	if( threadExitEvent == nullptr )
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
	//
	// get code see if its valid because we are polling
	// 99% of the time it will just return zero
	//

	auto const tempStart = std::chrono::steady_clock::now();
	auto const tempLast = end;

	ir_code const tempCode = BTV_GetRCCodeEx();

	auto const tempEnd = std::chrono::steady_clock::now();

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
