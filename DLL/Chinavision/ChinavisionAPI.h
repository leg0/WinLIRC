#ifndef ChinavisionAPI_H
#define ChinavisionAPI_H

#include <Windows.h>
#include "winusb.h"
#include <chrono>

class ChinavisionAPI {

public:

	ChinavisionAPI();

	BOOL init(HANDLE threadExit);
	void deinit();
	void threadProc();
	void waitTillDataIsReady(std::chrono::microseconds maxUSecs);
	int decodeCommand(char *out, size_t out_size);

private:

	void findDevice();
	void killThread	();
	void cleanUp	();
	
	//======================
	HANDLE	m_threadHandle;
	HANDLE	m_dataReadyEvent;
	HANDLE	m_threadExitEvent;
	HANDLE	m_exitEvent;

	LARGE_INTEGER m_time;
	LARGE_INTEGER m_lastTime;
	LARGE_INTEGER m_frequency;
	TCHAR m_deviceName[1024];		// should be enough space
	HANDLE m_deviceHandle;

	DWORD m_irCode;
	UCHAR m_inPipeId;
	WINUSB_INTERFACE_HANDLE m_usbHandle;
	//======================
};

#endif
