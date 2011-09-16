#ifndef RECEIVE_H
#define RECEIVE_H

#include <windows.h>
#include "LIRCDefines.h"

class Receive {

public:
	Receive();
   ~Receive();

	int		init(int deviceID);
	void	deinit();
	bool	getData(ir_code *out);
	bool	dataReady();
	void	callBackFunction(DWORD key);
	void	waitTillDataIsReady(int maxUSecs);
private:

	void	setData(ir_code data);

	//===================
	int	DevID;
	ir_code	dataBuffer[256];
	UCHAR	bufferStart;
	UCHAR	bufferEnd;
	//===================

};

#endif