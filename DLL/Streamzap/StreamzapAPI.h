#ifndef STREAMZAPAPI_H
#define STREAMZAPAPI_H

#include <Windows.h>
#include "LIRCDefines.h"
#include "irdata.h"

class StreamzapAPI {

public:

	StreamzapAPI();

	BOOL init();
	void deinit();
	void threadProc();
	void waitTillDataIsReady(int maxUSecs);
	bool dataReady();
	int	 decodeCommand(char *out);

private:

	void decode		(BYTE data, int numberOfBytes);
	void killThread	();

	//================
	HANDLE	threadHandle;
	BOOL	done;
	int		value;
	int		lastValue;
	int		repeat;
	int		repeatCount;
	int		numberOfBits;
	//================
};

#endif
