#include "Receive.h"
#include "Globals.h"
#include <stdio.h>
#include <tchar.h>
#include <winlirc-common/Win32Helpers.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

DWORD WINAPI IRReader(void *recieveClass) {

	((Receive*)recieveClass)->threadProc();
	return 0;
}

Receive::Receive() {
	threadHandle		= nullptr;
	m_pKsTunerPropSet	= nullptr;
	bufferStart			= 0;
	bufferEnd			= 0;
	last_key			= 0;
	repeats				= 0;

	CoInitializeEx(nullptr,COINIT_MULTITHREADED);
}

Receive::~Receive(){
	deinit();
	CoUninitialize();
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
		hr = pSysDevEnum->CreateClassEnumerator(KSCATEGORY_BDA_NETWORK_TUNER, &pEnumCat, 0);
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
					m_pKsTunerPropSet = nullptr;
					// query for interface
					hr = pFilter->QueryInterface(IID_IKsPropertySet, (void **)&m_pKsTunerPropSet);
					if (m_pKsTunerPropSet)
					{
						DWORD type_support;
						hr = m_pKsTunerPropSet->QuerySupported(KSPROPERTYSET_QBOXControl,
							KSPROPERTY_CTRL_IR,
							&type_support);
						if ( SUCCEEDED(hr) && (type_support & KSPROPERTY_SUPPORT_GET) )
							if (devCount == devNum)
								break;
					}
				}
				pMoniker.Release();
			}
		}
	}

	if ( FAILED(hr) || !m_pKsTunerPropSet )
		return 0;

	if FAILED(hr) return 0;

	threadHandle = CreateThread(nullptr,0,IRReader,(void *)this,0,nullptr);
	return threadHandle!=nullptr;
}

void Receive::deinit()
{
	KillThread(exitEvent,threadHandle);
	m_pKsTunerPropSet.Release();
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

	while(TRUE) {
		Sleep(100);
		ULONG bytesReturned = 0;
		QBOXCMD ir_data;

		// make call into driver
		HRESULT hr = m_pKsTunerPropSet->Get(KSPROPERTYSET_QBOXControl,
			KSPROPERTY_CTRL_IR,
			nullptr, 0,
			&ir_data, sizeof(ir_data),
			&bytesReturned);

		if FAILED(hr) break;

		if (WaitForSingleObject (exitEvent,0) == WAIT_OBJECT_0)
			break;

		if ((ir_data.rc_dev==0xff) || (ir_data.rc_key==0xff))
			continue;

		WORD code = MAKEWORD(ir_data.rc_key,ir_data.rc_dev);

#ifdef _DEBUG
		char code_str[128];
		sprintf_s(code_str,"TBS QBOX RC: 0x%04X\n",(DWORD)code);
		OutputDebugStringA(code_str);
#endif//RCCODE_DEBUGOUT
		
		setData(code);	// no conversion needed (hopefully)
		SetEvent(dataReadyEvent);
	}

	SAFE_CLOSE_HANDLE(exitEvent);

	CoUninitialize ();
}
