#ifndef RECEIVEDATA_H
#define RECEIVEDATA_H

#include "Serial.h"
#include <winlirc/PluginApi.h>
#include <chrono>

class SendReceiveData
{
public:
	SendReceiveData();

	bool	init();
	void	deinit();

	bool	getData(lirc_t *out);
	bool	dataReady();
	bool	waitTillDataIsReady(std::chrono::microseconds maxUSecs);
	void	threadProc();
	int		send(struct ir_remote *remote, struct ir_ncode *code, int repeats);
	
private:

	void	setData(lirc_t data);
	void	receiveLoop();
	UCHAR	calcPR2(int frequency);

	//==========================
	lirc_t		dataBuffer[256];
	UCHAR		bufferStart;
	UCHAR		bufferEnd;
	HANDLE		threadHandle;
	CSerial		serial;
	HANDLE		exitEvent;
	HANDLE		overlappedEvent;
	OVERLAPPED	overlapped;
	//==========================
};

#endif