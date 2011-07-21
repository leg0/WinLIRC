#include "StreamzapAPI.h"
#include <stdio.h>
#include "Globals.h"
#include "LIRCDefines.h"

DWORD WINAPI SZThread(void *recieveClass) {

	((StreamzapAPI*)recieveClass)->threadProc();
	return 0;
}

StreamzapAPI::StreamzapAPI() {

	threadHandle= NULL;
	done		= FALSE;

	bufferStart	= 0;
	bufferEnd	= 0;
	pulse		= PULSE_BIT;
	newSignal	= TRUE;

	QueryPerformanceFrequency(&frequency);
}

BOOL StreamzapAPI::init() {

	//=======
	BOOL ret;
	//=======

	ret = sz_Open();

	if(ret) {
		threadHandle = CreateThread(NULL,0,SZThread,(void *)this,0,NULL);
	}

	if(ret && threadHandle) return TRUE;

	return FALSE;
}

void StreamzapAPI::deinit() {

	killThread();

	threadHandle = NULL;
}

void StreamzapAPI::threadProc() {

	//==================
	DWORD	numberOfBytes;
	BYTE	data;
	//==================

	sz_Flush();	//flush any old data

	pulse = PULSE_BIT;

	//printf("entering receive loop\n");

	while(!done) {
		if(sz_ReadFile(&data,&numberOfBytes)) {
			decode(data,numberOfBytes);
		}
	}

	sz_Close();

	//printf("exited thread\n");

}

void StreamzapAPI::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==NULL) evt=1;
	else evt=2;

	if(!dataReady())
	{
		ResetEvent(dataReadyEvent);
		int res;
		if(maxUSecs)
			res=WaitForMultipleObjects(evt,events,FALSE,(maxUSecs+500)/1000);
		else
			res=WaitForMultipleObjects(evt,events,FALSE,INFINITE);
		if(res==(WAIT_OBJECT_0+1))
		{
			//DEBUG("Unknown thread terminating (readdata)\n");
			ExitThread(0);
			return;
		}
	}
}

void StreamzapAPI::killThread() {

	done = TRUE;

	if(threadHandle!=NULL) {

		//===========
		DWORD result;
		//===========

		result = 0;

		if(GetExitCodeThread(threadHandle,&result)==0) 
		{
			CloseHandle(threadHandle);
			threadHandle = NULL;
			return;
		}

		if(result==STILL_ACTIVE)
		{
			WaitForSingleObject(threadHandle,INFINITE);
		}

		CloseHandle(threadHandle);
		threadHandle = NULL;
	}
}

bool StreamzapAPI::dataReady() {

	if(bufferStart==bufferEnd) return false;
	
	return true;
}

void StreamzapAPI::decode(BYTE data, int numberOfBytes) {

	if(numberOfBytes!=0) {

		
		if(!data) {
			//
			// skip
			//
		}
		else if(data!=255) {

			if(newSignal) {

				//==========
				lirc_t temp;
				//==========

				QueryPerformanceCounter(&time);

				newSignal	= FALSE;
				temp		= (lirc_t)(((time.QuadPart - lastTime.QuadPart)*1000000) / frequency.QuadPart);
				lastTime	= time;

				temp += 100000;
				if(temp>PULSE_MASK) temp = PULSE_MASK;

				setData(temp);	// add this space maybe it'll help
				//printf("space time %i\n",temp);
			}

			setData(((data*256)+128)|pulse);

			

			if(pulse)	pulse = 0;
			else		pulse = PULSE_BIT;
		}
		else {

			pulse		= PULSE_BIT;
			newSignal	= TRUE;

			//printf("last signal received \n");

			QueryPerformanceCounter(&lastTime);

			SetEvent(dataReadyEvent);

			Sleep(10);
		}
	}

	else {
		Sleep(100);
	}
}

void StreamzapAPI::setData(lirc_t data) {

	//if(data&PULSE_BIT) printf("pulse %i\n",data&PULSE_MASK);
	//else printf("space %i\n",data);

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool StreamzapAPI::getData(lirc_t *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

