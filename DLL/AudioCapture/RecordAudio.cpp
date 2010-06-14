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

#include "RecordAudio.h"
#include <MMSystem.h>
#include <stdio.h>
#include <tchar.h>
#include "Globals.h"

#pragma comment(lib,"winmm.lib")

//
// this exists outside of this class (global)
//

void CALLBACK waveInProc(HWAVEIN hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);

RecordAudio::RecordAudio() {

	hWaveIn	= NULL;
}

RecordAudio::~RecordAudio() {

	closeAudioDevice();

	delete analyseAudio;
	analyseAudio = NULL;
}

int RecordAudio::calcBufferSize(int frequency){

	//
	// just calc a rough guide, stereo/mono halves/doubles value but not so concerned with that
	// If the buffer is too big, we'll have latency issues, too small could cause other probs
	//
	if(frequency==11025) return 1024;
	if(frequency==22050) return 2048;
	if(frequency==44100) return 4096;
	if(frequency==48000) return 4096;
	if(frequency==96000) return 8192;

	//
	// some unknown frequency
	//
	return 4096;
}

bool RecordAudio::startRecording(int deviceID, int frequency, int numberOfChannels, bool leftChannel) {

	//==============
	MMRESULT result;
	//==============

	openAudioDevice(deviceID,frequency,numberOfChannels,leftChannel);
	prepareBuffers(calcBufferSize(frequency));

	result = waveInStart(hWaveIn);

	if(result!=MMSYSERR_NOERROR) {

		//failed somehow
		//printf("failed\n");
		return false;
	}

	return true;

}

void RecordAudio::stopRecording() {

	closeAudioDevice();
}

void RecordAudio::openAudioDevice(int deviceID, int frequency, int numberOfChannels, bool leftChannel) {

	//======================
	WAVEFORMATEX waveFormat;
	//======================

	waveFormat.cbSize			= 0;
	waveFormat.wFormatTag		= WAVE_FORMAT_PCM;
	waveFormat.nChannels		= numberOfChannels;
	waveFormat.wBitsPerSample	= 8;
	waveFormat.nBlockAlign		= waveFormat.nChannels * waveFormat.wBitsPerSample/8;
	waveFormat.nSamplesPerSec	= frequency;
	waveFormat.nAvgBytesPerSec	= waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

	//
	//need to change this
	//

	MMRESULT result = waveInOpen(&hWaveIn,deviceID,&waveFormat,(DWORD_PTR)waveInProc,(DWORD_PTR)this,CALLBACK_FUNCTION);

	//printf("open result %i\n",result);

	analyseAudio = new AnalyseAudio(frequency,waveFormat.nChannels,leftChannel);
}

void RecordAudio::closeAudioDevice() {

	if(hWaveIn) {

		unPrepareBuffers();
		waveInClose(hWaveIn);

		hWaveIn = NULL;
	}
}

void RecordAudio::prepareBuffers(int bufferSize) {

	//==============
	MMRESULT	res;
	int			i;
	//==============

	res = 0;

	for(i=0;i<NUMBER_OF_BUFFERS; i++) {

		ZeroMemory(&waveHDR[i],sizeof(WAVEHDR));

		waveHDR[i].lpData			= (LPSTR) new UCHAR[bufferSize];
		waveHDR[i].dwBufferLength	= bufferSize;		// must be aligned with nBlockAlign
		waveHDR[i].dwUser			= i;

		res = waveInPrepareHeader(hWaveIn,&waveHDR[i],sizeof(WAVEHDR));

		if(res!=MMSYSERR_NOERROR) {
			//failed somehow
			//printf("failed somehow prepareheader\n");
		}

		res = waveInAddBuffer(hWaveIn,&waveHDR[i],sizeof(WAVEHDR));

		if(res!=MMSYSERR_NOERROR) {
			//failed somehow
			//printf("failed somehow waveInAddBuffer\n");
		}

		//printf("added buffer %i\n",i);
	}

}

void RecordAudio::unPrepareBuffers() {

	if(hWaveIn) {

		waveInStop(hWaveIn);

		for(int i=0; i<NUMBER_OF_BUFFERS; i++) {

			if(waveHDR[i].lpData) {

				waveInUnprepareHeader(hWaveIn,&waveHDR[i],sizeof(WAVEHDR));

				delete [] waveHDR[i].lpData;
			}

		}
	}

}

void RecordAudio::processHeader(WAVEHDR *waveHeader) {

	//printf("process header !\n");

	if(WHDR_DONE==(WHDR_DONE &waveHeader->dwFlags)) {

		//
		// process audio here identify spaces and pulses etc
		//
		analyseAudio->decodeData((UCHAR*)waveHeader->lpData,waveHeader->dwBytesRecorded);

		//
		// use buffer again for recording
		//
		waveInAddBuffer(hWaveIn,waveHeader,sizeof(WAVEHDR));
	}
}

void CALLBACK waveInProc(HWAVEIN hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	//printf("call back function %i\n",uMsg);
	switch(uMsg)
	{
		case WIM_CLOSE:
			break;

		case WIM_DATA:
			{
				//printf("data received\n");
				RecordAudio *recordAudio=(RecordAudio*)dwInstance;
				recordAudio->processHeader((WAVEHDR *)dwParam1);
			}
			break;

		case WIM_OPEN:
			break;

		default:
			break;
	}
}

