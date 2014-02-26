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
	threadHandle	= NULL;
	m_pKsVCPropSet	= NULL;
	bufferStart		= 0;
	bufferEnd		= 0;
	last_key		= 0;
	repeats			= 0;

	CoInitializeEx(NULL,COINIT_MULTITHREADED);
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
	CComPtr <ICreateDevEnum>	pSysDevEnum = NULL;	
	HRESULT hr = pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	if (hr == S_OK)
	{
		// create a class enumerator for the desired category defined by classGuid.
		CComPtr <IEnumMoniker> pEnumCat = NULL;	//moniker enumerator for filter categories
		hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
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
					m_pKsVCPropSet = NULL;
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

	threadHandle = CreateThread(NULL,0,IRReader,(void *)this,0,NULL);
	return threadHandle!=NULL;
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

	CoInitializeEx(NULL,COINIT_MULTITHREADED);

	exitEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

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
		OutputDebugString(code_str);
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
