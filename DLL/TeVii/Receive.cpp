#include "Receive.h"
#include "Globals.h"
#include "TeVii.h"
#include <stdio.h>
#include <tchar.h>

Receive::Receive() {
	DevID = -1;
	bufferStart		= 0;
	bufferEnd		= 0;
}

Receive::~Receive() {

	deinit();
}

void __stdcall callBackFunc(void* pContext, DWORD key) {

	if(pContext != NULL) {
		((Receive*)pContext)->callBackFunction(key);
	}
}

int Receive::init(int deviceID) {
	if (DevID!=-1) return 0;
	if ( deviceID > (FindDevices()-1) ) return 0;
	if (!OpenDevice(deviceID, NULL, NULL)) return 0;

	if (!SetRemoteControl(deviceID,&callBackFunc,this))
	{
		CloseDevice(deviceID);
		return 0;
	}

	DevID = deviceID;
	return 1;
}

void Receive::deinit() {

	if (DevID!=-1)
		CloseDevice(DevID);
	DevID = -1;
}

void Receive::setData(ir_code data) {

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool Receive::dataReady() {

	if(bufferStart==bufferEnd) return false;
	
	return true;
}

bool Receive::getData(ir_code *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

void Receive::callBackFunction(DWORD key) {
	setData(key);		// no conversion needed (hopefully)
	SetEvent(dataReadyEvent);
}

void Receive::waitTillDataIsReady(int maxUSecs) {

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
