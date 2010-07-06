#ifndef STREAMZAPAPI_H
#define STREAMZAPAPI_H

#include <Windows.h>
#include "LIRCDefines.h"
#include "irdata.h"

class StreamzapAPI {

public:

	StreamzapAPI();

	BOOL init();
	void deinit();
	void threadProc();
	void waitTillDataIsReady(int maxUSecs);
	bool getData(lirc_t *out);
	bool dataReady	();

private:

	void decode		(BYTE data, int numberOfBytes);
	void killThread	();
	void setData	(lirc_t data);
	
	//======================
	HANDLE	threadHandle;
	BOOL	done;
	lirc_t	dataBuffer[256];
	UCHAR	bufferStart;
	UCHAR	bufferEnd;
	lirc_t	pulse;

	LARGE_INTEGER time;
	LARGE_INTEGER lastTime;
	LARGE_INTEGER frequency;
	BOOL newSignal;
	//======================
};

#endif
