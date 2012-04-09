#ifndef RECEIVE_H
#define RECEIVE_H

#include <windows.h>
#include "LIRCDefines.h"

#include "bdaiface.h"
#include "bdatif.h"
#include "ks.h"
#include "ksproxy.h"
#include "stdio.h"
#include <ks.h>
#include <ksmedia.h>
#include <bdamedia.h>
#include "atlbase.h" //needed for string conversions
#include "dshow.h"

#include "GPIO_Interface.h"
#include "Interrupt_Interface.h"
#include "tmIIR_Decoder.h"
#include "OSDependFactory.h"

class Receive : public CIR_DecoderCallback
{

public:
	Receive();
   ~Receive();

	int		init(int devNum, tmIrDecoderId tDecoderId = IR_DECODER_ID_NEC_32, unsigned minRepeat = 0);
	void	deinit();
	bool	getData(ir_code *out);
	bool	dataReady();
	void	IR_DecoderCallback(tmIrDecoderId Id, PVOID pContext);
	void	waitTillDataIsReady(int maxUSecs);
private:

	void	setData(ir_code data);

	CComPtr <IKsControl> m_pIKsControl;
	WORD	m_wImplementationId;
	BYTE	m_ucGpio;
	CInterrupt_Interface* m_pInterruptLayer;
	CGPIO_Interface*      m_pGpioLayer;
	COSDependFactory      tFactory;
	tmIIrDecoder*         m_pIrDecoder;
	ULONG m_minRepeat;
	DWORD last_key, repeats;
	//===================
	ir_code	dataBuffer[256];
	UCHAR	bufferStart;
	UCHAR	bufferEnd;
	//===================
};

#endif