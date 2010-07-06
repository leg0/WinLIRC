#include "StreamzapAPI.h"
#include <stdio.h>
#include "Globals.h"
#include "LIRCDefines.h"

DWORD WINAPI SZThread(void *recieveClass) {

	((StreamzapAPI*)recieveClass)->threadProc();
	return 0;
}

StreamzapAPI::StreamzapAPI() {

	threadHandle= NULL;
	done		= FALSE;
	value		= 0;
	repeat		= 0;
	repeatCount	= 0;
	numberOfBits= 0;
	lastValue	= 0;
}

BOOL StreamzapAPI::init() {

	//=======
	BOOL ret;
	//=======

	ret = sz_Open();

	if(ret) {
		threadHandle = CreateThread(NULL,0,SZThread,(void *)this,0,NULL);
	}

	if(ret && threadHandle) return TRUE;

	return FALSE;
}

void StreamzapAPI::deinit() {

	killThread();

	threadHandle = NULL;
}

void StreamzapAPI::threadProc() {

	//==================
	DWORD	numberOfBytes;
	BYTE	data;
	//==================

	numberOfBytes	= 0;
	value			= 0;
	repeat			= 0;
	repeatCount		= 0;
	numberOfBits	= 0;
	lastValue		= 0;

	sz_Flush();	//flush any old data

	//printf("entering receive loop\n");

	while(!done) {
		if(sz_ReadFile(&data,&numberOfBytes)) {
			decode(data,numberOfBytes);
		}
	}

	sz_Close();

	//printf("exited thread\n");

}

void StreamzapAPI::waitTillDataIsReady(int maxUSecs) {

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

void StreamzapAPI::killThread() {

	done = TRUE;

	if(threadHandle!=NULL) {

		//===========
		DWORD result;
		//===========

		result = 0;

		if(GetExitCodeThread(threadHandle,&result)==0) 
		{
			threadHandle = NULL;
			return;
		}

		if(result==STILL_ACTIVE)
		{
			WaitForSingleObject(threadHandle,INFINITE);
			threadHandle = NULL;
		}
	}
}

bool StreamzapAPI::dataReady() {

	if(value==0) return false;

	return true;
}

void StreamzapAPI::decode(BYTE data, int numberOfBytes) {

	if(numberOfBytes!=0) {

		//================
		unsigned int temp;
		//================

		if(data<5) {
			temp = 0;
			temp  <<= numberOfBits;
			value  |= temp;
			numberOfBits++;
		}
		else if(data<10) {
			temp = 1;
			temp  <<= numberOfBits;
			value  |= temp;
			numberOfBits++;
		}
		else if(data==255){

			if(value & 0xC) {

				value &= (~12);
				value >>= 4;

				if(repeat==1 && lastValue==value) {
					repeatCount++;
				}
				else {
					repeatCount = 0;
				}

				repeat = 1;
			}
			else {

				value &= (~63);
				value >>= 6;

				if(repeat==0 && lastValue==value) {
					repeatCount++;
				}
				else {
					repeatCount = 0;
				}

				repeat = 0;
			}

			lastValue = value;

			SetEvent(dataReadyEvent);

			Sleep(100);
		}
	}

	else {
		Sleep(100);
	}
}

int StreamzapAPI::decodeCommand(char *out) {

	//==================
	char buttonName[12];
	//==================

	switch(value) {

		case 0x121:		strcpy_s(buttonName,_countof(buttonName),"0");	break;
		case 0x20121:	strcpy_s(buttonName,_countof(buttonName),"1");	break;
		case 0x18121:	strcpy_s(buttonName,_countof(buttonName),"2");	break;
		case 0x8121:	strcpy_s(buttonName,_countof(buttonName),"3");	break;
		case 0x6121:	strcpy_s(buttonName,_countof(buttonName),"4");	break;
		case 0xe121:	strcpy_s(buttonName,_countof(buttonName),"5");	break;
		case 0x12121:	strcpy_s(buttonName,_countof(buttonName),"6");	break;
		case 0x2121:	strcpy_s(buttonName,_countof(buttonName),"7");	break;
		case 0x1921:	strcpy_s(buttonName,_countof(buttonName),"8");	break;
		case 0x9921:	strcpy_s(buttonName,_countof(buttonName),"9");	break;

		case 0x7921:	strcpy_s(buttonName,_countof(buttonName),"POWER");	break;
		case 0x3921:	strcpy_s(buttonName,_countof(buttonName),"MUTE");	break;

		case 0x4921:	strcpy_s(buttonName,_countof(buttonName),"CH_UP");	break;
		case 0x10921:	strcpy_s(buttonName,_countof(buttonName),"CH_DOWN");break;

		case 0xc921:	strcpy_s(buttonName,_countof(buttonName),"VOL_UP");	break;
		case 0x921:		strcpy_s(buttonName,_countof(buttonName),"VOL_DOWN");break;

		case 0x721:		strcpy_s(buttonName,_countof(buttonName),"UP");		break;
		case 0x1f21:	strcpy_s(buttonName,_countof(buttonName),"DOWN");	break;
		case 0x8721:	strcpy_s(buttonName,_countof(buttonName),"LEFT");	break;
		case 0x2721:	strcpy_s(buttonName,_countof(buttonName),"RIGHT");	break;
		case 0x6721:	strcpy_s(buttonName,_countof(buttonName),"OK");		break;

		case 0x3f21:	strcpy_s(buttonName,_countof(buttonName),"MENU");	break;
		case 0x4f21:	strcpy_s(buttonName,_countof(buttonName),"EXIT");	break;

		case 0xf21:		strcpy_s(buttonName,_countof(buttonName),"PLAY");	break;
		case 0x1321:	strcpy_s(buttonName,_countof(buttonName),"PAUSE");	break;
		case 0x9321:	strcpy_s(buttonName,_countof(buttonName),"STOP");	break;

		case 0x7321:	strcpy_s(buttonName,_countof(buttonName),"|<<");	break;
		case 0x3321:	strcpy_s(buttonName,_countof(buttonName),">>|");	break;
		case 0x4321:	strcpy_s(buttonName,_countof(buttonName),"RECORD");	break;

		case 0xc321:	strcpy_s(buttonName,_countof(buttonName),"<<");	break;
		case 0x10321:	strcpy_s(buttonName,_countof(buttonName),">>");	break;

		case 0x421:		strcpy_s(buttonName,_countof(buttonName),"RED");	break;
		case 0x20421:	strcpy_s(buttonName,_countof(buttonName),"GREEN");	break;
		case 0x18421:	strcpy_s(buttonName,_countof(buttonName),"YELLOW");	break;
		case 0x8421:	strcpy_s(buttonName,_countof(buttonName),"BLUE");	break;

		default: return 0;
	}

	_snprintf_s(out,PACKET_SIZE+1,PACKET_SIZE+1,"%016llx %02x %s %s\n",__int64(0),repeatCount,buttonName,"Streamzap");

	value			= 0;
	numberOfBits	= 0;
	
	return 1;
}

