#include "AnalyseAudio.h"
#include <Math.h>
#include <stdio.h>
#include "Globals.h"

//
// Heavily based on hw_audio_alsa.c from the LIRC project
//

#define PULSE_BIT  0x01000000
#define PULSE_MASK 0x00FFFFFF

AnalyseAudio::AnalyseAudio(int frequency, int numberOfChannels, bool leftChannel) {

	signalMax			= 0x80;
	signalMin			= 0x80;
	signalLevel			= 0;

	previousSample		= 0x80;

	multiplyConstant	= 1000000 / (double)frequency;
	maxCount			= (~0) / (DWORD)multiplyConstant;

	sampleCount			= 0;
	signalState			= 0;

	numberOfChans		= numberOfChannels ;
	this->leftChannel	= leftChannel;

	//
	//basic error checking
	//
	if(numberOfChans<1)	numberOfChans = 1;
	if(numberOfChans>2)	numberOfChans = 2;

	bufferStart		= 0;
	bufferEnd		= 0;

	//dataTest();

	pulse = false;
}

bool AnalyseAudio::crossedVirtualMiddle(int cs, int ps, int middle) {

	//===========
	int crossed;
	int crossed1;
	//===========

	crossed		= ps - middle;
	crossed1	= cs - middle;

	if(crossed<0 && crossed1<0) return false;
	if(crossed>0 && crossed1>0) return false;

	return true;
}

void AnalyseAudio::decodeData(UCHAR *data, int bytesRecorded) {

	//=======================
	UCHAR currentSample;
	UCHAR signalMiddle;
	UCHAR absoluteDifference;
	UCHAR sl;
	bool  crossedZeroPoint;
	//=======================

	for(int i=0; i<bytesRecorded; i+=numberOfChans) {

		if(numberOfChans>1 && !leftChannel) {
			currentSample = data[i+1];
		}
		else {
			currentSample = data[i];
		}

		if(sampleCount > maxCount) {
			pulse = false;
			sampleCount = 0;
		}

		else {

			if(currentSample > (128 + 16)) {

				if(!pulse) {
					//changing from space to pulse so add this
				}
				pulse = true;
			}
			else {

				if(pulse) {
					//changing from pulse to space so add this
				}
				pulse = false;
			}

		}


		setData(x);

		SetEvent(dataReadyEvent);	//signal data is ready for other thread

		signalState ^= PULSE_BIT;


		sampleCount++;
		


	}

	//printf("signalmin %i signalmax %i CS %i SL %i SM %i\n",signalMin,signalMax,currentSample,signalLevel,signalMiddle);
	//printf("signal level %i\n",signalLevel);
}

bool AnalyseAudio::getData(UINT *out) {

	if(!dataReady()) waitTillDataIsReady(0);;

	*out = dataBuffer[bufferStart];
	//printf("buffer value %i\n",*out);

	bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

void AnalyseAudio::setData(UINT data) {

	dataBuffer[bufferEnd] = data;
	bufferEnd++;
}

bool AnalyseAudio::dataReady() {

	if(bufferStart==bufferEnd) return false;
	
	return true;
}

void AnalyseAudio::dataTest() {

	setData(1979421);
	setData(16786296);
	setData(4486);
	setData(16777808);
	setData(540);
	setData(16777811);
	setData(519);
	setData(16777808);
	setData(1659);
	setData(16777832);
	setData(542);
	setData(16777784);
	setData(544);
	setData(16777810);
	setData(542);
	setData(16777809);
	setData(541);
	setData(16777785);
	setData(546);
	setData(16777811);
	setData(1678);
	setData(16777788);
	setData(1678);
	setData(16777810);
	setData(540);
	setData(16777787);
	setData(1678);
	setData(16777787);
	setData(1680);
	setData(16777813);
	setData(1678);
	setData(16777788);
	setData(1675);
	setData(16777786);
	setData(1683);
	setData(16777812);
	setData(1676);
	setData(16777786);
	setData(540);
	setData(16777811);
	setData(1687);
	setData(16777777);
	setData(1679);
	setData(16777787);
	setData(545);
	setData(16777809);
	setData(542);
	setData(16777815);
	setData(1673);
	setData(16777796);
	setData(537);
	setData(16777812);
	setData(515);
	setData(16777809);
	setData(1679);
	setData(16777811);
	setData(540);
	setData(16777787);
	setData(566);
	setData(16777763);
	setData(1702);
	setData(16777811);
	setData(1656);
	setData(16777809);
	setData(540);
	setData(16777811);
	setData(1658);
	setData(16777810);
	setData(40061);
	setData(16786287);
	setData(2225);
	setData(16777810);

}