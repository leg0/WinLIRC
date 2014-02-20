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
	threadHandle = NULL;
	m_pKsTunerPropSet = NULL;
	bufferStart	= 0;
	bufferEnd = 0;
	last_key = repeats = 0;	
}

Receive::~Receive(){
	deinit();
}

int Receive::init(int devNum, unsigned minRepeat)
{	
	m_minRepeat = minRepeat;
	deinit();

	CoInitialize(NULL);
	int devCount=0;

	// create system device enumerator
	CComPtr <ICreateDevEnum>	pSysDevEnum = NULL;	
	HRESULT hr = pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	if (hr == S_OK)
	{
		// create a class enumerator for the desired category defined by classGuid.
		CComPtr <IEnumMoniker> pEnumCat = NULL;	//moniker enumerator for filter categories
		hr = pSysDevEnum->CreateClassEnumerator(KSCATEGORY_BDA_NETWORK_TUNER, &pEnumCat, 0);
		if (hr == S_OK)
		{
			// reset the enumeration
			pEnumCat->Reset();

			// now iterate through enumeration
			ULONG cFetched = 0;
			CComPtr <IMoniker> pMoniker = NULL;

			// get a pointer to each moniker
			while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
			{
				CComPtr <IBaseFilter> pFilter = NULL;
				// create an instance of the filter
				hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter);
				if (hr == S_OK)
				{							
					m_pKsTunerPropSet = NULL;
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

	threadHandle = CreateThread(NULL,0,IRReader,(void *)this,0,NULL);
	return threadHandle!=NULL;
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

bool Receive::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==NULL) evt=1;
	else evt=2;

	if(!dataReady())
	{
		ResetEvent(dataReadyEvent);
		int res;
		if(maxUSecs)
			res=WaitForMultipleObjects(evt,events,FALSE,(maxUSecs+500)/1000);
		else
			res=WaitForMultipleObjects(evt,events,FALSE,INFINITE);
		if(res==(WAIT_OBJECT_0+1))
		{
			return false;
		}
	}

	return true;
}

void Receive::threadProc() {

	exitEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	while(TRUE) {
		Sleep(100);
		ULONG bytesReturned = 0;
		QBOXCMD ir_data;

		// make call into driver
		HRESULT hr = m_pKsTunerPropSet->Get(KSPROPERTYSET_QBOXControl,
			KSPROPERTY_CTRL_IR,
			NULL, 0,
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
		OutputDebugString(code_str);
#endif//RCCODE_DEBUGOUT
		
		setData(code);	// no conversion needed (hopefully)
		SetEvent(dataReadyEvent);
	}

	SAFE_CLOSE_HANDLE(exitEvent);
}
