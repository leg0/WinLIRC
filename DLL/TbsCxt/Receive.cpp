#include "Receive.h"
#include "Globals.h"
#include <stdio.h>
#include <tchar.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

DWORD WINAPI IRReader(void *recieveClass) {

	((Receive*)recieveClass)->threadProc();
	return 0;
}

Receive::Receive() {
	threadHandle = NULL;
	m_pKsVCPropSet = NULL;
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
	killThread();
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

void Receive::waitTillDataIsReady(int maxUSecs) {

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
			//DEBUG("Unknown thread terminating (readdata)\n");
			ExitThread(0);
			return;
		}
	}
}

void Receive::threadProc() {

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

	if(exitEvent) {
		CloseHandle(exitEvent);
		exitEvent = NULL;
	}
}

void Receive::killThread() {
	//
	// need to kill thread here
	//
	if(exitEvent) {
		SetEvent(exitEvent);
	}

	if(threadHandle!=NULL) {

		//===========
		DWORD result;
		//===========

		result = 0;

		if(GetExitCodeThread(threadHandle,&result)==0) 
		{
			CloseHandle(threadHandle);
			threadHandle = NULL;
			return;
		}

		if(result==STILL_ACTIVE)
		{
			WaitForSingleObject(threadHandle,INFINITE);
		}

		CloseHandle(threadHandle);
		threadHandle = NULL;
	}
}
