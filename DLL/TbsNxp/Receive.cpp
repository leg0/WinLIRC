#include "Receive.h"
#include "Globals.h"
#include <stdio.h>
#include <tchar.h>

Receive::Receive() {
	m_pIKsControl = NULL;
	m_pInterruptLayer = NULL;
	m_pGpioLayer = NULL;
	m_pIrDecoder = NULL;
	m_wImplementationId = 0x7160;
	m_ucGpio = 4;
	bufferStart	= 0;
	bufferEnd = 0;
	last_key = repeats = 0;	
}

Receive::~Receive(){
	deinit();
}

int Receive::init(int devNum, tmIrDecoderId tDecoderId, unsigned minRepeat)
{	
	m_minRepeat = minRepeat;
	deinit();

	CoInitialize(NULL);
	int devCount=0;
	CComPtr <IBaseFilter> pFilter = NULL;

	// create system device enumerator
	CComPtr <ICreateDevEnum> pSysDevEnum = NULL;	
	HRESULT hr = pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	if (hr == S_OK)
	{
		// create a class enumerator for the desired category defined by classGuid.
		CComPtr <IEnumMoniker> pEnumCat = NULL;	//moniker enumerator for filter categories
		hr = pSysDevEnum->CreateClassEnumerator(CLSID_DeviceControlCategory, &pEnumCat, 0);
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
				//get a pointer to property bag (which has filter)
				CComPtr <IPropertyBag> pPropBag = NULL;	
				if (pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag) != S_OK)
					break;

				// create an instance of the filter
				hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter);
				if (hr == S_OK)
				{							
					TCHAR szFriendlyName[MAX_PATH];
					VARIANT varName;
					// retrieve the friendly name of the filter
					VariantInit(&varName);
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					WideCharToMultiByte(CP_ACP, 0, varName.bstrVal, -1, szFriendlyName, sizeof(szFriendlyName), 0, 0);
					VariantClear(&varName);
					if ( strstr(szFriendlyName,"TBS") || strstr(szFriendlyName,"TT-budget") )
						if (devCount == devNum)
							break;
				}
				pMoniker.Release();
			}
		}
	}
	if (pFilter == NULL) return 0;

	m_pIKsControl = NULL;
	// query for interface    
    hr = pFilter->QueryInterface(IID_IKsControl, (void **)&m_pIKsControl);

	if ( FAILED(hr) || !m_pIKsControl )
		return 0;

	m_pGpioLayer = new CGPIO_Interface( m_pIKsControl, m_wImplementationId );
	if (!m_pGpioLayer) return 0;
	m_pInterruptLayer = new CInterrupt_Interface( m_pIKsControl, m_wImplementationId );
	if (!m_pInterruptLayer) return 0;

	if ( m_pGpioLayer->tmISetGpioConfiguration( 1 << m_ucGpio, 1 << m_ucGpio ) != TM_OK) return 0;

	tmIrHwInfo_t tHwInfo;

	tHwInfo.dwGpioNr           = m_ucGpio;
	tHwInfo.tDecoderId         = tDecoderId;
	tHwInfo.tInterruptType     = FALLING_EDGE_ACTIVATED;
	tHwInfo.bInterruptDecoding = TRUE;

	m_pIrDecoder = new tmIIrDecoder( this, m_pGpioLayer, m_pInterruptLayer, &tFactory, &tHwInfo );
	if (!m_pIrDecoder) return 0;
	return 1;
}

void Receive::deinit()
{
	if (m_pIrDecoder)
	{
		delete m_pIrDecoder;
		m_pIrDecoder = NULL;
	}

	if (m_pInterruptLayer)
	{
		delete m_pInterruptLayer;
		m_pInterruptLayer = NULL;
	}

	if (m_pGpioLayer)
	{
		delete m_pGpioLayer;
		m_pGpioLayer = NULL;
	}

	m_pIKsControl.Release();
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

void Receive::IR_DecoderCallback(tmIrDecoderId Id, PVOID ptData)
{
	DWORD key=0;
	switch ( Id )
	{  
	case IR_DECODER_ID_NEC_32:
		{		
			tmIrDecoderNec32Data_t* pNecData = (tmIrDecoderNec32Data_t*)ptData;
			key = MAKEWORD(pNecData->Data, pNecData->Address);
			if (pNecData->Repeat && key == last_key)
				if (repeats < m_minRepeat)
				{
					repeats++;
					return;
				}
			repeats = 0;
			last_key = key;
			break;
		}
	case IR_DECODER_ID_NEC_40:
		{
			tmIrDecoderNec40Data_t* pNecData = (tmIrDecoderNec40Data_t*)ptData;
			key = MAKEWORD(pNecData->Data, pNecData->Address);
			if (pNecData->Repeat && key == last_key)
				if (repeats < m_minRepeat)
				{
					repeats++;
					return;
				}
			repeats = 0;
			last_key = key;
			break;
		}
	case IR_DECODER_ID_RC5:
		{
			tmIrDecoderRC5Data_t* pNecData = (tmIrDecoderRC5Data_t*)ptData;
			key = 0x3000 | (DWORD)pNecData->Toggle<<11 | (DWORD)pNecData->Group<<6 | pNecData->Command;			
			break;
		}
	case IR_DECODER_ID_RC6:
		{
			tmIrDecoderRC6Data_t* pNecData = (tmIrDecoderRC6Data_t*)ptData;
			key = pNecData->Header<<16 | pNecData->Information;
			break;
		}
	default:
		return;
	}

	setData(key);		// no conversion needed (hopefully)
	SetEvent(dataReadyEvent);
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
