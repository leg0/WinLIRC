#include "Receive.h"
#include "Globals.h"
#include "LWExtDLL.h"
#include <stdio.h>
#include <tchar.h>
#include <winlirc-common/Win32Helpers.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

DWORD WINAPI GNTIRReader(void *recieveClass) {

	((Receive*)recieveClass)->threadProc();
	return 0;
}

Receive::Receive() {
	threadHandle = NULL;
	bufferStart		= 0;
	bufferEnd		= 0;
}

Receive::~Receive() {
	deinit();
}

char const* gnt_ids[] = {
	"USB\\VID_0572&PID_8695",
	"USB\\VID_0572&PID_8696",
	"USB\\VID_0572&PID_8698",
	"USB\\VID_0572&PID_8616",
	"USB\\VID_0572&PID_86D6",
	"USB\\VID_0572&PID_D685",
	"USB\\VID_0572&PID_D688",
	"USB\\VID_0572&PID_8606",
	"USB\\VID_0572&PID_D320",
	"USB\\VID_0572&PID_6886",
	"USB\\VID_0572&PID_C680",
	"USB\\VID_0572&PID_C682",
	"USB\\VID_0572&PID_D811",
	"USB\\VID_1B80&PID_E450",
	"USB\\VID_1B80&PID_E388",
	"USB\\VID_1F4D&PID_2000",
	"USB\\VID_1F4D&PID_3000",
	"USB\\VID_1F4D&PID_3100",
	"USB\\VID_1F4D&PID_8000",
	"USB\\VID_1F4D&PID_D675",
	"USB\\VID_1F4D&PID_C200",
	"USB\\VID_1FE1&PID_5456",
	"USB\\VID_1FE1&PID_5457",
	"USB\\VID_0CCD&PID_00A8",
	"USB\\VID_0CCD&PID_00B0",
	"USB\\VID_0572&PID_C682",
	"USB\\VID_1F4D&PID_D220",
	"USB\\VID_0572&PID_C686",
};

int Receive::init() {
	//if (LWEXT_Open(0x8696)!=DLL_OK) return 0;
    auto x = &fscanf; // msvc fails to link without this. says it can't find fscanf.
	int rc=DLL_ERROR;
	for (int i=0; i < _countof(gnt_ids); i++)
	{
		rc = LWEXT_OpenEx(const_cast<char*>(gnt_ids[i]));
		if (rc==DLL_OK)
		{
			OutputDebugString("\nGeniatech device ID: ");
			OutputDebugString(gnt_ids[i]);
			OutputDebugString("\n");
			break;
		}
	}

	if (rc!=DLL_OK) return FALSE;

	threadHandle = CreateThread(NULL,0,GNTIRReader,(void *)this,0,NULL);

	return threadHandle!=NULL;
}

void Receive::deinit() {
	KillThread(exitEvent,threadHandle);
	LWEXT_Close();
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
		ULONG ulCmd, ulAddr;
		if (LWEXT_GetIr(&ulCmd,&ulAddr)!=DLL_OK){
			//break;
		}

		if (WaitForSingleObject (exitEvent,0) == WAIT_OBJECT_0)
			break;

		if ((ulCmd==0xff) || (ulCmd==0xcc) || (ulAddr==0xff) || (ulAddr==0xcc))
			continue;

		WORD code = MAKEWORD(ulCmd,ulAddr);
		code |= 0x3000;

#ifdef _DEBUG
		char code_str[128];
		sprintf(code_str,"Geniatech RC: 0x%04X\n",(DWORD)code);
		OutputDebugString(code_str);
#endif//RCCODE_DEBUGOUT

		setData(code);	// no conversion needed (hopefully)
		SetEvent(dataReadyEvent);
		Sleep(100);
	}

	SAFE_CLOSE_HANDLE(exitEvent);
}