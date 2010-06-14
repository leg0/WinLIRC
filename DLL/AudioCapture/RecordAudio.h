/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 2010 Ian Curtis
 */

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
