#include "Receive.h"
#include "Globals.h"
#include <stdio.h>
#include <tchar.h>
#include "../Common/Win32Helpers.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

DWORD WINAPI IRReader(void *recieveClass) {

	((Receive*)recieveClass)->threadProc();
	return 0;
}

Receive::Receive() {
	threadHandle	= nullptr;
	m_pKsVCPropSet	= nullptr;
	bufferStart		= 0;
	bufferEnd		= 0;
	last_key		= 0;
	repeats			= 0;

	CoInitializeEx(nullptr,COINIT_MULTITHREADED);
}

Receive::~Receive(){
	deinit();
	CoUninitialize ();
}

int Receive::init(int devNum, unsigned minRepeat)
{	
	m_minRepeat = minRepeat;
	deinit();

	int devCount=0;

	// create system device enumerator
	CComPtr <ICreateDevEnum>	pSysDevEnum = nullptr;	
	HRESULT hr = pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	if (hr == S_OK)
	{
		// create a class enumerator for the desired category defined by classGuid.
		CComPtr <IEnumMoniker> pEnumCat = nullptr;	//moniker enumerator for filter categories
		hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
		if (hr == S_OK)
		{
			// reset the enumeration
			pEnumCat->Reset();

			// now iterate through enumeration
			ULONG cFetched = 0;
			CComPtr <IMoniker> pMoniker = nullptr;

			// get a pointer to each moniker
			while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
			{
				CComPtr <IBaseFilter> pFilter = nullptr;
				// create an instance of the filter
				hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter);
				if (hr == S_OK)
				{							
					m_pKsVCPropSet = nullptr;
					// query for interface
					hr = pFilter->QueryInterface(IID_IKsPropertySet, (void **)&m_pKsVCPropSet);
					if (m_pKsVCPropSet)
					{
						DWORD type_support;
						hr = m_pKsVCPropSet->QuerySupported(KSPROPSETID_CustomIRCaptureProperties,
							KSPROPERTY_IRCAPTURE_COMMAND,
							&type_support);
						if ( SUCCEEDED(hr) && (type_support & KSPROPERTY_SUPPORT_SET) )
							if (devCount == devNum)
								break;
					}
				}
				pMoniker.Release();
			}
		}
	}

	if ( FAILED(hr) || !m_pKsVCPropSet )
		return 0;

	KSPROPERTY_IRCAPTURE_COMMAND_S ir_cmd;
	ir_cmd.CommandCode = IRCAPTURE_COMMAND_START;

	// make call into driver
	hr = m_pKsVCPropSet->Set(KSPROPSETID_CustomIRCaptureProperties,
		KSPROPERTY_IRCAPTURE_COMMAND,
		&ir_cmd, sizeof(ir_cmd),
		&ir_cmd, sizeof(ir_cmd));

	if FAILED(hr) return 0;

	threadHandle = CreateThread(nullptr,0,IRReader,(void *)this,0,nullptr);
	return threadHandle!=nullptr;
}

void Receive::deinit()
{
	KillThread(exitEvent,threadHandle);
	m_pKsVCPropSet.Release();
}

void Receive::setData(ir_code data) {

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool Receive::dataReady() {

	if(bufferStart==bufferEnd) return false;
	
	return true;
}

bool Receive::getData(ir_code *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

bool Receive::waitTillDataIsReady(std::chrono::microseconds maxUSecs) {

	HANDLE const events[2]={dataReadyEvent,threadExitEvent};
	DWORD const evt = (threadExitEvent == nullptr) ? 1 : 2;

	if(!dataReady())
	{
		ResetEvent(dataReadyEvent);
		using namespace std::chrono;
		DWORD const dwTimeout = maxUSecs > 0us
			? duration_cast<milliseconds>(maxUSecs + 500us).count()
			: INFINITE;
		DWORD const res = ::WaitForMultipleObjects(evt, events, false, dwTimeout);
		if(res==(WAIT_OBJECT_0+1))
		{
			return false;
		}
	}

	return true;
}

void Receive::threadProc() {

	CoInitializeEx(nullptr,COINIT_MULTITHREADED);

	exitEvent = CreateEvent(nullptr,TRUE,FALSE,nullptr);

	BYTE addr=0, cmd=0, count=0;

	while(TRUE) {
		Sleep(50);
		ULONG bytesReturned = 0;
		KSPROPERTY_IRCAPTURE_KEYSTROKES_S kStore = {1,1};
		KSPROPERTY ks;
		ks.Id = KSPROPERTY_IRCAPTURE_KEYSTROKES;

		// make call into driver
		HRESULT hr = m_pKsVCPropSet->Get(KSPROPSETID_CustomIRCaptureProperties,
			KSPROPERTY_IRCAPTURE_KEYSTROKES,
			&ks, sizeof(ks),
			&kStore, sizeof(kStore),
			&bytesReturned);

		if FAILED(hr) break;

		if (WaitForSingleObject (exitEvent,0) == WAIT_OBJECT_0)
			break;

		if ((kStore.dwAddress==0xffff) || (kStore.dwCommand==0xffff) || (kStore.dwCommand==0xff) || (kStore.dwCommand==0x00))
			continue;

		if (kStore.dwCommand==0x10000)
			count++;
		else
		{
			addr = (BYTE)kStore.dwAddress;
			cmd = (BYTE)kStore.dwCommand;
			count=0;
		}

		if (count && count<5)
			continue;

		WORD code = MAKEWORD(cmd,addr);

#ifdef _DEBUG
		char code_str[128];
		sprintf_s(code_str,"TBS CXT RC: 0x%04X\n",(DWORD)code);
		OutputDebugStringA(code_str);
#endif//RCCODE_DEBUGOUT
		
		setData(code);	// no conversion needed (hopefully)
		SetEvent(dataReadyEvent);
	}

	KSPROPERTY_IRCAPTURE_COMMAND_S ir_cmd;
	ir_cmd.CommandCode = IRCAPTURE_COMMAND_STOP;

	// make call into driver
	m_pKsVCPropSet->Set(KSPROPSETID_CustomIRCaptureProperties,
		KSPROPERTY_IRCAPTURE_COMMAND,
		&ir_cmd, sizeof(ir_cmd),
		&ir_cmd, sizeof(ir_cmd));

	SAFE_CLOSE_HANDLE(exitEvent);

	CoUninitialize ();
}
