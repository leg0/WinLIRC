#ifndef __IR_DEVICE_LIST_H_INCLUDED__
#define __IR_DEVICE_LIST_H_INCLUDED__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <setupapi.h>
#include <vector>
#include <string>

class IrDeviceList 
{
public:
	typedef std::vector<std::wstring> StringList;

	IrDeviceList();
	StringList & get();

protected:
	void MceIrOpenUsbDevice(LPGUID  pGuid );

	StringList m_List;
};

#endif