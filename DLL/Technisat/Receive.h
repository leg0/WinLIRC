#pragma once

#include <windows.h>
#include <winlirc/WLPluginAPI.h>
#include <Dshow.h>
#include <initguid.h>
#include "b2c2_defs.h"
#include "B2C2_Guids.h"
#include "ib2c2mpeg2tunerctrl.h"
#include "ib2c2mpeg2datactrl.h"
#include "ib2c2mpeg2avctrl.h"
#include <chrono>

class Receive {

public:
	Receive();
   ~Receive();

	int		init(int deviceID);
	int		enumDevices() { return m_dwDeviceCount; }
	void	deinit();
	bool	getData(ir_code *out);
	bool	dataReady();
	bool	waitTillDataIsReady(std::chrono::microseconds maxUSecs);
	void	threadProc();
private:
	IBaseFilter				*m_pFilter;
	IB2C2MPEG2TunerCtrl4	*m_pIB2C2MPEG2TunerCtrl;	
	IB2C2MPEG2DataCtrl6		*m_pIB2C2MPEG2DataCtrl;	
	IB2C2MPEG2AVCtrl3		*m_pIB2C2MPEG2AvCtrl;	
	DWORD m_dwDeviceCount;

	void	setData(ir_code data);
	HANDLE	threadHandle;
	HANDLE	exitEvent;
	//===================
	int	DevID;
	ir_code	dataBuffer[256];
	UCHAR	bufferStart;
	UCHAR	bufferEnd;
	//===================

};
