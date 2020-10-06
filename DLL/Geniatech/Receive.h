#ifndef RECEIVE_H
#define RECEIVE_H

#include <windows.h>
#include <winlirc/winlirc_api.h>

class Receive {

public:
	Receive();
   ~Receive();

	int		init();
	void	deinit();
	bool	getData(ir_code *out);
	bool	dataReady();
	bool	waitTillDataIsReady(int maxUSecs);
	void	threadProc();
private:
	void	setData(ir_code data);

	//======================
	HANDLE	threadHandle;
	HANDLE	exitEvent;
	//======================
	ir_code	dataBuffer[256];
	UCHAR	bufferStart;
	UCHAR	bufferEnd;
	//======================

};

#endif