#ifndef RECEIVE_H
#define RECEIVE_H

#include <windows.h>
#include "ttusbirapiexp.h"
#include <winlirc/PluginApi.h>

class Receive {

public:
	Receive();
   ~Receive();

	int		init(int deviceID, int busyLED, int powerLED);
	void	deinit();
	bool	getData(lirc_t *out);
	bool	dataReady();
	void	callBackFunction(PVOID Buf, ULONG len, USBIR_MODES IRMode, HANDLE hOpen, BYTE DevIdx);
	bool	waitTillDataIsReady(int maxUSecs);

private:

	void	setData(lirc_t data);

	//===================
	HANDLE	deviceHandle;
	lirc_t	dataBuffer[256];
	UCHAR	bufferStart;
	UCHAR	bufferEnd;
	//===================
};

#endif