/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
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
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * Modifications Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#include "stdafx.h"
#include "irdriver.h"
#include "Globals.h"
#include <tchar.h>
#include "LIRCDefines.h"
#include "Send.h"
	
unsigned int IRThread(void *drv) {
	((CIRDriver *)drv)->ThreadProc();
	return 0;
}

CIRDriver::CIRDriver()
{
	hPort			= NULL;
	ov.hEvent		= NULL;
	IRThreadHandle	= NULL;
	hDataReadyEvent	= CreateEvent(NULL,FALSE,FALSE,NULL);
	
	bufferStart		= 0;
	bufferEnd		= 0;
}

CIRDriver::~CIRDriver()
{
	//DEBUG("~CIRDriver\n");
	ResetPort();
	KillThread(&IRThreadHandle,&IRThreadEvent);
	if(hDataReadyEvent) CloseHandle(hDataReadyEvent);
}

bool CIRDriver::InitPort()
{
	KillThread(&IRThreadHandle,&IRThreadEvent);

	bufferStart	= 0;
	bufferEnd	= 0;
	
	if(ov.hEvent)
	{
		SetEvent(ov.hEvent);	// singal it
		Sleep(100);				// wait a tiny bit
		CloseHandle(ov.hEvent);	// and close it
		ov.hEvent=NULL;
	}

	if(hPort)
	{
		SetCommMask(hPort,0);	// stop any waiting on the port
		Sleep(100);				// wait a tiny bit
		CloseHandle(hPort);		// and close it
		hPort=NULL;
	}

	if((hPort=CreateFile(
		settings.port,GENERIC_READ | GENERIC_WRITE,
		0,0,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0))==INVALID_HANDLE_VALUE)
	{
		hPort=NULL;
		return false;
	}

	DCB dcb;
	if(!GetCommState(hPort,&dcb))
	{
		CloseHandle(hPort);
		hPort=NULL;
		return false;
	}
	if (settings.animax) dcb.fDtrControl=DTR_CONTROL_ENABLE; //set DTR high, the animax receiver needs this for power
	else
		dcb.fDtrControl=DTR_CONTROL_DISABLE; // set the transmit LED to off initially.
	dcb.fRtsControl=RTS_CONTROL_ENABLE;

	dcb.BaudRate = _tstoi(settings.speed);
	devicetype = settings.deviceType;				
	virtpulse = settings.virtualPulse;		

	if(!SetCommState(hPort,&dcb))
	{
		CloseHandle(hPort);
		hPort=NULL;
		//DEBUG("SetCommState failed.\n");
		return false;
	}

	SetTransmitPort(hPort,settings.transmitterType);

	if(settings.sense==-1)
	{
		/* Wait for receiver to settle (since we just powered it on) */
		Sleep(1000);
		DWORD state;
		if(!GetCommModemStatus(hPort,&state))
		{
			CloseHandle(hPort);
			hPort=NULL;
			return false;
		}
		sense=(state & MS_RLSD_ON) ? 1 : 0;
		//DEBUG("Sense set to %d\n",sense);
	}
	else
		sense = settings.sense;

	if((ov.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL))==NULL)
	{
		CloseHandle(hPort);
		hPort=NULL;
		return false;
	}

	/* Start the thread */
	/* THREAD_PRIORITY_TIME_CRITICAL combined with the REALTIME_PRIORITY_CLASS */
	/* of this program results in the highest priority (26 out of 31) */
	if((IRThreadHandle=
		AfxBeginThread(IRThread,(void *)this,THREAD_PRIORITY_TIME_CRITICAL))==NULL)
	{
		CloseHandle(hPort);
		CloseHandle(ov.hEvent);
		hPort=ov.hEvent=NULL;
		return false;
	}

	//DEBUG("Port initialized.\n");
	
	return true;
}

void CIRDriver::ResetPort(void)
{
	//DEBUG("Resetting port\n");
	
	KillThread(&IRThreadHandle,&IRThreadEvent);
	
	if(ov.hEvent) {
		CloseHandle(ov.hEvent);
		ov.hEvent=NULL;
	}
	if(hPort) {
		CloseHandle(hPort);
		hPort=NULL;
	}
}

void CIRDriver::ThreadProc(void)
{
	/* Virtually no error checking is done here, because */
	/* it's pretty safe to assume that everything works, */
	/* and we have nowhere to report errors anyway.      */

	/* We use two timers in case the high resolution doesn't   */
	/* last too long before wrapping around (is that true?     */
	/* is it really only a 32 bit timer or a true 64 bit one?) */

	__int64 hr_time, hr_lasttime, hr_freq;	// high-resolution
	time_t lr_time, lr_lasttime;			// low-resolution

	DWORD status;
	GetCommModemStatus(hPort, &status);
	int prev=(status & MS_RLSD_ON) ? 1 : 0;

	/* Initialize timer stuff */
	QueryPerformanceFrequency((LARGE_INTEGER *)&hr_freq);

	/* Get time (both LR and HR) */
	time(&lr_lasttime);
	QueryPerformanceCounter((LARGE_INTEGER *)&hr_lasttime);
	
	HANDLE events[2]={ov.hEvent,IRThreadEvent};

	for(;;)
	{
		/* We want to be notified of DCD or RX changes */
		if(SetCommMask(hPort, devicetype ? EV_RLSD : EV_RXCHAR)==0)	
		{
			//DEBUG("SetCommMask returned zero, error=%d\n",GetLastError());
		}
		/* Reset the event */
		ResetEvent(ov.hEvent);
		/* Start waiting for the event */
		DWORD event;
		if(WaitCommEvent(hPort,&event,&ov)==0 && GetLastError()!=997)
		{
			//DEBUG("WaitCommEvent error: %d\n",GetLastError());
		}

		/* Wait for the event to get triggered */
		int res=WaitForMultipleObjects(2,events,FALSE,INFINITE);
		
		/* Get time (both LR and HR) */
		QueryPerformanceCounter((LARGE_INTEGER *)&hr_time);
		time(&lr_time);
		
		if(res==WAIT_FAILED)
		{
			//DEBUG("Wait failed.\n");
			continue;
		}
		
		if(res==(WAIT_OBJECT_0+1))
		{
			//DEBUG("IRThread terminating\n");
			AfxEndThread(0);
			return;
		}
		
		if(res!=WAIT_OBJECT_0)
		{
			//DEBUG("Wrong object\n");
			continue;
		}

		int dcd;
		if (devicetype) {				
			GetCommModemStatus(hPort,&status);

			dcd = (status & MS_RLSD_ON) ? 1 : 0;

			if(dcd==prev)
			{
				/* Nothing changed?! */
				/* Continue without changing time */
				continue;
			}

			prev=dcd;
		}

		int deltv=(int)(lr_time-lr_lasttime);
		if (devicetype && (deltv>15)) {		
			/* More than 15 seconds passed */
			deltv=0xFFFFFF;
			if(!(dcd^sense))
			{
				/* sense had to be wrong */
				sense=sense?0:1;
				//DEBUG("sense was wrong!\n");
			}
		} else
			deltv=(int)(((hr_time-hr_lasttime)*1000000) / hr_freq);
	
		lr_lasttime=lr_time;
		hr_lasttime=hr_time;
		
		int data;				
		if (devicetype) {		
			data = (dcd^sense) ? (deltv) : (deltv | 0x1000000);	

			setData(data);
			SetEvent(hDataReadyEvent);
		} else {
			data = deltv;	

			setData(data-100);						
			setData(virtpulse | 0x1000000);
			SetEvent(hDataReadyEvent);			
			PurgeComm(hPort,PURGE_RXCLEAR);			
		}
	}
}

void CIRDriver::setData(UINT data) {

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool CIRDriver::dataReady() {

	if(bufferStart==bufferEnd) return false;
	
	return true;
}

bool CIRDriver::getData(UINT *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

unsigned long CIRDriver::readData(unsigned long maxusec)
{
	UINT x=0;

	waitTillDataIsReady(maxusec);

	getData(&x);

	return x;
}

void CIRDriver::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={hDataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==NULL) evt=1;
	else evt=2;

	if(!dataReady())
	{
		ResetEvent(hDataReadyEvent);
		int res;
		if(maxUSecs)
			res=WaitForMultipleObjects(evt,events,FALSE,(maxUSecs+500)/1000);
		else
			res=WaitForMultipleObjects(evt,events,FALSE,INFINITE);
		if(res==(WAIT_OBJECT_0+1))
		{
			//DEBUG("Unknown thread terminating (readdata)\n");
			AfxEndThread(0);
			return;
		}
	}
}

