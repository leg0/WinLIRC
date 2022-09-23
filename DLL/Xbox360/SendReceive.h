#pragma once

#include <Windows.h>
#include "XInput.h"
#include <chrono>

class SendReceive {

public:

	SendReceive();

	BOOL init					(HANDLE exit);
	void deinit					();
	void threadProc				();
	bool waitTillDataIsReady	(std::chrono::microseconds maxUSecs);
	bool dataReady				();
	int	 decodeCommand			(char *out, size_t out_size);

private:

	//================================
	XINPUT_STATE	m_controllerState;
	HANDLE			m_threadHandle;
	HANDLE			m_threadExitEvent;
	HANDLE			m_dataReadyEvent;
	BOOL			m_done;
	int				m_value;
	int				m_repeats;
	//================================
};
