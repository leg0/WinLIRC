#pragma once


#include <winsock2.h>
#include <Windows.h>
#include "../Common/Socket.h"
#include <winlirc/WLPluginAPI.h>
#include <chrono>

class Server {

public:
	Server();

	int		init();
	void	deinit();
	void	threadProc();
	bool	getData(lirc_t *out);
	bool	dataReady();
	bool	waitTillDataIsReady(std::chrono::microseconds maxUSecs);

private:

	void	processData();
	void	setData(lirc_t data);

	winlirc::Socket server;
	HANDLE		threadHandle;
	HANDLE		exitThread;
	WSAEVENT	wEvent;
	lirc_t		dataBuffer[256];
	UCHAR		bufferStart;
	UCHAR		bufferEnd;
};
