#include "Server.h"
#include "Globals.h"
#include <stdio.h>
#include "../Common/Win32Helpers.h"

DWORD WINAPI ServerThread(void *server) {

	((Server*)server)->threadProc();
	return 0;
}

Server::Server() {

	threadHandle	= nullptr;
	wEvent			= WSA_INVALID_EVENT;
	bufferStart		= 0;
	bufferEnd		= 0;
}

int Server::init() {

	//=============================
	WSADATA				data;
	struct sockaddr_in	serv_addr;
	//=============================

	if(WSAStartup(MAKEWORD(2, 0),&data)) {
		//printf("failed startup\n");
		return 0;		// win sock version not supported
	}

	server.reset(socket(AF_INET, SOCK_DGRAM, 0));

	if(!server) {
		//printf("failed socket\n");
		return 0;	//failed
	}

	memset(&serv_addr,0,sizeof(struct sockaddr_in));

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	serv_addr.sin_port			= htons(8765);

	if(bind(server.get(),(sockaddr *)&serv_addr,sizeof(serv_addr))==SOCKET_ERROR) {
		//printf("failed bind\n");
		return 0;
	}

	wEvent = WSACreateEvent();
	WSAEventSelect(server.get(), wEvent, FD_READ);

	exitThread		= CreateEvent(nullptr,FALSE,FALSE,nullptr);
	threadHandle	= CreateThread(nullptr,0,ServerThread,(void *)this,0,nullptr);

	if(threadHandle==nullptr) {
		//printf("failed thread\n");
		return 0;		//thread creation failure
	}

	return 1;
}

void Server::deinit() {

	KillThread(exitThread,threadHandle);

	server.reset();
	SAFE_CLOSE_HANDLE(exitThread);

	WSACleanup();
}

void Server::threadProc() {

	//===============
	HANDLE events[2];
	int res;
	//===============

	events[0] = wEvent;
	events[1] = exitThread;

	while(1) {

		res = WaitForMultipleObjects(2,events,FALSE,INFINITE);

		if(res==(WAIT_OBJECT_0)) {

			//============================
			WSANETWORKEVENTS networkEvent;
			//============================

			processData();

			WSAEnumNetworkEvents(server.get(), wEvent, &networkEvent);
		}
		else if(res==(WAIT_OBJECT_0+1)) {
			break;	//exit thread !
		}
		else {
			break;	//exit thread !
		}
	}
}

void Server::processData() {

	//====================
	int		numberOfBytes;
	CHAR	buffer[8192];
	//====================

	numberOfBytes = recv(server.get(), buffer, sizeof(buffer), 0);

	if(numberOfBytes>0 && !(numberOfBytes%2)) {

		//================
		UCHAR	packed[2];
		UINT	tmp;
		lirc_t	data;
		//================

		for(int i=0; i<numberOfBytes; i+=2) {

			packed[0] = buffer[i+0];
			packed[1] = buffer[i+1];

			// assumes active low - why ? :o
			data = (packed[1] & 0x80) ? 0 : PULSE_BIT;

			// Convert 1/16384-seconds to microseconds
			tmp = (((UINT)packed[1])<<8) | packed[0];

			// tmp = ((tmp & 0x7FFF) * 1000000) / 16384;
			// prevent integer overflow:
			tmp = ((tmp & 0x7FFF) * 15625) / 256;

			data |= tmp & PULSE_MASK;

			setData(data);
		}

		SetEvent(dataReadyEvent);
	}
}

void Server::setData(lirc_t data) {

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool Server::dataReady() {

	if(bufferStart==bufferEnd) return false;
	
	return true;
}

bool Server::getData(lirc_t *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

bool Server::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==nullptr) evt=1;
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
			return false;
		}
	}

	return true;
}
