#ifndef RECORDAUDIO_H
#define RECORDAUDIO_H

#include <Windows.h>

class RecordAudio {

public:
	RecordAudio();
   ~RecordAudio();

	bool startRecording		(int deviceID, int frequency, int numberOfChannels, bool leftChannel);		//device ID and format .. eventually
	void stopRecording		();

	void processHeader		(WAVEHDR *waveHeader);

private:

	void openAudioDevice		(int deviceID, int frequency, int numberOfChannels, bool leftChannel);
	void closeAudioDevice		();
	void prepareBuffers			(int bufferSize);
	void unPrepareBuffers		();
	int	 calcBufferSize			(int frequency);
	

	#define NUMBER_OF_BUFFERS 3

	HWAVEIN hWaveIn;
	WAVEHDR	waveHDR[NUMBER_OF_BUFFERS];

};

#endif
