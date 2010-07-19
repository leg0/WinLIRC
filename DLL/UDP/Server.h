#ifndef SERVER_H
#define SERVER_H


#include <winsock2.h>
#include <Windows.h>
#include "LIRCDefines.h"

class Server {

public:
	Server();

	int		init();
	void	deinit();
	void	threadProc();
	bool	getData(lirc_t *out);
	bool	dataReady();
	void	waitTillDataIsReady(int maxUSecs);

private:

	void	processData();
	void	setData(lirc_t data);

	SOCKET		server;
	HANDLE		threadHandle;
	HANDLE		exitThread;
	WSAEVENT	wEvent;
	lirc_t		dataBuffer[256];
	UCHAR		bufferStart;
	UCHAR		bufferEnd;
};

#endif