#include "Receive.h"
#include "Globals.h"

Receive::Receive() {

	bufferStart		= 0;
	bufferEnd		= 0;
	deviceHandle	= INVALID_HANDLE_VALUE;
}

Receive::~Receive() {

	deinit();
}

void callBackFunc(PVOID Context, PVOID Buf, ULONG len, USBIR_MODES IRMode, HANDLE hOpen, BYTE DevIdx) {

	if(Context != NULL) {
		((Receive*)Context)->callBackFunction(Buf, len, IRMode, hOpen, DevIdx);
	}
}

int Receive::init(int deviceID, int busyLED, int powerLED) {

	deviceHandle = irOpen(deviceID, USBIR_MODE_DIV, &callBackFunc, this);

	if(deviceHandle==INVALID_HANDLE_VALUE) return 0;

	if(busyLED)	irSetBusyLEDFreq(deviceHandle, 128);
	else		irSetBusyLEDFreq(deviceHandle, 0);

	if(powerLED)irSetPowerLED	(deviceHandle, TRUE);
	else		irSetPowerLED	(deviceHandle, FALSE);

	return 1;
}

void Receive::deinit() {

	if(deviceHandle!=INVALID_HANDLE_VALUE) {

		irSetBusyLEDFreq(deviceHandle, FALSE);
		irSetPowerLED	(deviceHandle, FALSE);
		irClose			(deviceHandle);

		deviceHandle = INVALID_HANDLE_VALUE;
	}
}

void Receive::setData(lirc_t data) {

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool Receive::dataReady() {

	if(bufferStart==bufferEnd) return false;
	
	return true;
}

bool Receive::getData(lirc_t *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

void Receive::callBackFunction(PVOID Buf, ULONG len, USBIR_MODES IRMode, HANDLE hOpen, BYTE DevIdx) {

	//=============
	lirc_t *buffer;
	//=============

	//
	// santity checks
	//
	if(hOpen==INVALID_HANDLE_VALUE) return;
	if(IRMode!=USBIR_MODE_DIV)		return;

	buffer = (lirc_t*)Buf;

	if(len>256) len = 256;		// to fit our buffer but if it sends data too infrequently will be severe key press lag anyway 

	for(unsigned int i=0; i<len; i++) {
		setData(buffer[i]);		// no conversion needed (hopefully)
	}

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