#ifndef STREAMZAPAPI_H
#define STREAMZAPAPI_H

#include <Windows.h>
#include "../Common/LIRCDefines.h"

class StreamzapAPI {

public:

	StreamzapAPI();

	BOOL init		(HANDLE exit);
	void deinit		();
	void threadProc	();
	bool getData	(lirc_t *out);
	bool dataReady	();
	void findDevice	();
	void waitTillDataIsReady(int maxUSecs);

private:

	void decode		(BYTE data);
	void killThread	();
	void setData	(lirc_t data);
	
	//======================
	bool	exitThread;
	HANDLE	threadHandle;
	HANDLE	threadExitEvent;
	HANDLE	dataReadyEvent;

	lirc_t	dataBuffer[256];
	UCHAR	bufferStart;
	UCHAR	bufferEnd;
	lirc_t	pulse;
	bool	nextByteFull;

	LARGE_INTEGER time;
	LARGE_INTEGER lastTime;
	LARGE_INTEGER frequency;
	bool newSignal;
	TCHAR deviceName[1024];		// should be enough space
	HANDLE deviceHandle;
	//======================
};

#endif
