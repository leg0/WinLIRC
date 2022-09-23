#pragma once

#include <Windows.h>
#include <winlirc/WLPluginAPI.h>
#include <chrono>

class StreamzapAPI {

public:

	StreamzapAPI();

	BOOL init		(HANDLE exit);
	void deinit		();
	void threadProc	();
	bool getData	(lirc_t *out);
	bool dataReady	();
	bool waitTillDataIsReady(std::chrono::microseconds maxUSecs);

private:

	void findDevice	();
	void decode		(BYTE data);
	void setData	(lirc_t data);
	
	//======================
	bool	m_exitThread;
	HANDLE	m_threadHandle;
	HANDLE	m_threadExitEvent;
	HANDLE	m_dataReadyEvent;

	lirc_t	m_dataBuffer[256];
	UCHAR	m_bufferStart;
	UCHAR	m_bufferEnd;
	lirc_t	m_pulse;
	bool	m_nextByteFull;
	bool	m_newSignal;

	LARGE_INTEGER m_time;
	LARGE_INTEGER m_lastTime;
	LARGE_INTEGER m_frequency;
	
	TCHAR	m_deviceName[1024];		// should be enough space
	HANDLE	m_deviceHandle;
	//======================
};
