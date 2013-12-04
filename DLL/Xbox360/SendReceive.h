#ifndef STREAMZAPAPI_H
#define STREAMZAPAPI_H

#include <Windows.h>
#include "../Common/LIRCDefines.h"
#include "XInput.h"

class SendReceive {

public:

	SendReceive();

	BOOL init					(HANDLE exit);
	void deinit					();
	void threadProc				();
	bool waitTillDataIsReady	(int maxUSecs);
	bool dataReady				();
	int	 decodeCommand			(char *out);

private:

	void killThread	();

	//==============================
	XINPUT_STATE	controllerState;
	HANDLE			threadHandle;
	HANDLE			threadExitEvent;
	HANDLE			dataReadyEvent;
	BOOL			done;
	int				value;
	int				repeats;
	//==============================
};

#endif
