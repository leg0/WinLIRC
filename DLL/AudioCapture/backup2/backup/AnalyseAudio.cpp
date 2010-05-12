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

		//if(i==0) printf("current sampled l %i r %i\n",data[i],data[i+1]);

		signalMiddle		= (signalMin + signalMax) / 2;

		if (currentSample <= signalMiddle)	signalMin = (signalMin * 7 + currentSample) / 8;
		if (currentSample >= signalMiddle)	signalMax = (signalMax * 7 + currentSample) / 8;
		
		absoluteDifference	= abs(currentSample - signalMiddle);
		signalLevel			= (signalLevel * 7 + absoluteDifference) / 8;

		//
		// Don't let too low signal levels as it makes us sensible to noise
		//
		sl = signalLevel;
		if (sl < 16) sl = 16;

		crossedZeroPoint = crossedVirtualMiddle(currentSample,previousSample,signalMiddle);

		//
		// Detect significant signal change
		//
		if ((abs (currentSample - signalMiddle) > sl/2) && crossedZeroPoint) {

			//=====
			UINT x;
			//=====

			if(sampleCount > maxCount) {
				x = PULSE_MASK;
				sampleCount = 0;
			}
			else {
				//===========
				double delta;
				//===========

				delta = (((double)signalMiddle - (double)currentSample)) / ((double)currentSample - (double)previousSample);

				x = (int)((sampleCount + delta) * multiplyConstant);

				sampleCount = -delta;
			}

			//
			//Consider impossible pulses with length greater than
			//0.02 seconds, thus it is a space (desynchronization).
			//
			if ((x > 20000) && signalState) {
				signalState = 0;
				//system("cls");
			}

			printf("x value %i\n",x);

			x |= signalState;

			//printf("x value %i\n",x);

			setData(x);

			SetEvent(dataReadyEvent);	//signal data is ready for other thread

			signalState ^= PULSE_BIT;
		}

		previousSample = currentSample;

		sampleCount += 1; 
	}

	printf("signalmin %i signalmax %i CS %i SL %i SM %i\n",signalMin,signalMax,currentSample,signalLevel,signalMiddle);
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