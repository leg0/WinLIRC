#ifndef RECEIVE_H
#define RECEIVE_H

#include <windows.h>
#include "../Common/LIRCDefines.h"

class Receive {

public:
	Receive();
   ~Receive();

	int		init(int deviceID);
	void	deinit();
	bool	getData(ir_code *out);
	bool	dataReady();
	void	callBackFunction(DWORD key);
	bool	waitTillDataIsReady(int maxUSecs);
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