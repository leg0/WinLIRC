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

	// LIRC features
	//=========================
	char			device[16];
	int				fd;
	unsigned long	features;
	unsigned long	send_mode;
	unsigned long	rec_mode;
	unsigned long	code_length;
	char			name[16];
	unsigned int	resolution;
	//=========================

private:

	void setData(UINT data);

	void dataTest();

	double	multiplyConstant; 
	double	sampleCount;
	DWORD	maxCount;
	DWORD	numberOfChans;
	bool	leftChannel;

	bool pulse;

	UINT dataBuffer[256];
	UCHAR bufferStart;
	UCHAR bufferEnd;
};

#endif