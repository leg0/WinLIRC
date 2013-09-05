#include "Receive.h"
#include "./ttbdadrvapi/ttBdaDrvApi.h"
#include "Globals.h"

Receive::Receive() {

	bufferStart		= 0;
	bufferEnd		= 0;
	deviceHandle	= INVALID_HANDLE_VALUE;
}

Receive::~Receive() {

	deinit();
}

void callBackFunc(PVOID pContext, DWORD *pBuf) {

	if(pContext && pBuf) ((Receive*)pContext)->callBackFunction(*pBuf);
}

int Receive::init(int DevCat, int deviceID) {

	deviceHandle = bdaapiOpen ((DEVICE_CAT)DevCat, deviceID);

	if(deviceHandle==INVALID_HANDLE_VALUE) return 0;

	if ( bdaapiOpenIR(deviceHandle,&callBackFunc,this) != RET_SUCCESS )
	{
		bdaapiClose(deviceHandle);
		return 0;
	}

	return 1;
}

void Receive::deinit() {

	if(deviceHandle!=INVALID_HANDLE_VALUE) {
		bdaapiClose(deviceHandle);
		deviceHandle = INVALID_HANDLE_VALUE;
	}
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