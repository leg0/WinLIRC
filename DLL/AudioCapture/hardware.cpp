#include "Globals.h"
#include "hardware.h"
#include "Decode.h"

struct hardware hw;

lirc_t readData(lirc_t timeout) {

	//==========
	lirc_t data;
	//==========

	if(!analyseAudio) return 0;

	waitTillDataIsReady(timeout);

	if(analyseAudio->getData((UINT*)&data)) {
		
		return data;
	}

	return 0;
}

int data_ready() {

	if(!analyseAudio) return 0;

	if(analyseAudio->dataReady()) return 1;

	return 0;
}

void initHardwareStruct() {

	hw.decode_func	= &receive_decode;
	hw.readdata		= &readData;
	hw.wait_for_data= &waitTillDataIsReady;
	hw.data_ready	= &data_ready;

	hw.features		= LIRC_CAN_REC_MODE2;
	hw.send_mode	= 0;
	hw.rec_mode		= LIRC_MODE_MODE2;
	hw.code_length	= 0;
	hw.resolution	= 0;

	strcpy(hw.device,"hw");
	strcpy(hw.name,"audio");
}