#ifndef RECEIVE_H
#define RECEIVE_H

#include <windows.h>
#include "../Common/LIRCDefines.h"

#include "bdaiface.h"
#include "bdatif.h"
#include "ks.h"
#include "ksproxy.h"
#include "stdio.h"
#include <ks.h>
#include <ksmedia.h>
#include <bdamedia.h>
#include <atlbase.h> //needed for string conversions
#include <dshow.h>

const GUID KSPROPSETID_CustomIRCaptureProperties = 
{ 0xb51c4994, 0x54, 0x4749, { 0x82, 0x43, 0x2, 0x9a, 0x66, 0x86, 0x36, 0x36 }};

#define IRCAPTURE_COMMAND_START        1
#define IRCAPTURE_COMMAND_STOP         2
#define IRCAPTURE_COMMAND_FLUSH        3

typedef struct 
{   
    DWORD             dwAddress;
    DWORD             dwCommand;  

} KSPROPERTY_IRCAPTURE_KEYSTROKES_S, *PKSPROPERTY_IRCAPTURE_KEYSTROKES_S;

typedef struct 
{    
    CHAR             CommandCode;    

} KSPROPERTY_IRCAPTURE_COMMAND_S, *PKSPROPERTY_IRCAPTURE_COMMAND_S;

typedef enum
{
    KSPROPERTY_IRCAPTURE_KEYSTROKES         = 0,
    KSPROPERTY_IRCAPTURE_COMMAND            = 1

}KSPROPERTY_IRCAPTURE_PROPS;

class Receive
{

public:
	Receive();
   ~Receive();

	int		init(int devNum, unsigned minRepeat = 0);
	void	deinit();
	bool	getData(ir_code *out);
	bool	dataReady();
	bool	waitTillDataIsReady(int maxUSecs);
	void	threadProc();
private:

	void	setData(ir_code data);
	void	killThread();
	HANDLE	threadHandle;
	HANDLE	exitEvent;

	CComPtr <IKsPropertySet>	m_pKsVCPropSet;
	ULONG m_minRepeat;
	DWORD last_key, repeats;
	//===================
	ir_code	dataBuffer[256];
	UCHAR	bufferStart;
	UCHAR	bufferEnd;
	//===================

};

#endif