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

DEFINE_GUIDSTRUCT( "C6EFE5EB-855A-4f1b-B7AA-87B5E1DC4113", KSPROPERTYSET_QBOXControl );
#define KSPROPERTYSET_QBOXControl DEFINE_GUIDNAMED( KSPROPERTYSET_QBOXControl )

typedef enum
{
	KSPROPERTY_CTRL_IR = 1,	
} KSPROPERTY_QBOXControl;

typedef struct {
	BYTE reserved[30];
	BYTE rc_dev;
	BYTE rc_dev1;
	BYTE rc_key;
	BYTE rc_key1;
	BYTE reserved1[254];
} QBOXCMD, *PQBOXCMD;

class Receive
{

public:
	Receive();
   ~Receive();

	int		init(int devNum, unsigned minRepeat = 0);
	void	deinit();
	bool	getData(ir_code *out);
	bool	dataReady();
	void	waitTillDataIsReady(int maxUSecs);
	void	threadProc();
private:

	void	setData(ir_code data);
	void	killThread();
	HANDLE	threadHandle;
	HANDLE	exitEvent;

	CComPtr <IKsPropertySet>	m_pKsTunerPropSet;
	ULONG m_minRepeat;
	DWORD last_key, repeats;
	//===================
	ir_code	dataBuffer[256];
	UCHAR	bufferStart;
	UCHAR	bufferEnd;
	//===================

};

#endif