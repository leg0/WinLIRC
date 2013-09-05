#ifndef RECEIVE_H
#define RECEIVE_H

#include <windows.h>
#include "../Common/LIRCDefines.h"

class Receive {

public:
	Receive();
   ~Receive();

	int		init();
	void	deinit();
	bool	getData(ir_code *out);
	bool	dataReady();
	void	waitTillDataIsReady(int maxUSecs);
	void	threadProc();
private:
	void	setData(ir_code data);
	void	killThread();
	HANDLE	threadHandle;
	HANDLE	exitEvent;
	//===================
	ir_code	dataBuffer[256];
	UCHAR	bufferStart;
	UCHAR	bufferEnd;
	//===================

};

#endif