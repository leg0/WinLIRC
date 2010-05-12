#ifndef ANALYSEAUDIO_H
#define ANALYSEAUDIO_H

#include <Windows.h>

//
// only accept 8bit mono/stere audio for now. Sampling frequency can change
//

class AnalyseAudio {

public:
	AnalyseAudio(int frequency, int numberOfChannels, bool leftChannel);

	void decodeData(UCHAR *data, int bytesRecorded);
	bool getData(UINT *out);
	bool dataReady();

private:

	bool crossedVirtualMiddle(int cs, int ps, int middle);
	void setData(UINT data);

	void dataTest();

	double	multiplyConstant; 
	double	sampleCount;
	DWORD	maxCount;
	DWORD	signalState;
	DWORD	numberOfChans;
	bool	leftChannel;

	UCHAR signalMax;
	UCHAR signalMin;
	UCHAR signalLevel;

	UCHAR previousSample;

	UINT dataBuffer[256];
	UCHAR bufferStart;
	UCHAR bufferEnd;
};

#endif