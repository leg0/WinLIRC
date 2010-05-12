#include "AudioFormats.h"
#include <MMSystem.h>

bool AudioFormats::formatSupported(int format) {

	//ignore all 16 bit formats

	if(format & WAVE_FORMAT_1M08) return true;
	if(format & WAVE_FORMAT_1S08) return true;

	if(format & WAVE_FORMAT_2M08) return true;
	if(format & WAVE_FORMAT_2S08) return true;

	if(format & WAVE_FORMAT_4M08) return true;
	if(format & WAVE_FORMAT_4S08) return true;

	if(format & WAVE_FORMAT_48M08) return true;
	if(format & WAVE_FORMAT_48S08) return true;

	if(format & WAVE_FORMAT_96M08) return true;
	if(format & WAVE_FORMAT_96S08) return true;

	return false;
}

void AudioFormats::getFormatString(int format, TCHAR *outString, int noBuffElements) {

	if(!noBuffElements) return;

	if(format & WAVE_FORMAT_1M08) { _tcscpy_s(outString,noBuffElements,_T("11.025 kHz, Mono, 8-bit")); return; }
	if(format & WAVE_FORMAT_1S08) { _tcscpy_s(outString,noBuffElements,_T("11.025 kHz, Stereo, 8-bit")); return; }

	if(format & WAVE_FORMAT_2M08) { _tcscpy_s(outString,noBuffElements,_T("22.05 kHz, Mono, 8-bit")); return; }
	if(format & WAVE_FORMAT_2S08) { _tcscpy_s(outString,noBuffElements,_T("22.05 kHz, Stereo, 8-bit")); return; }

	if(format & WAVE_FORMAT_4M08) { _tcscpy_s(outString,noBuffElements,_T("44.1 kHz, Mono, 8-bit")); return; }
	if(format & WAVE_FORMAT_4S08) { _tcscpy_s(outString,noBuffElements,_T("44.1 kHz, Stereo, 8-bit")); return; }

	if(format & WAVE_FORMAT_48M08) { _tcscpy_s(outString,noBuffElements,_T("48 kHz, Mono, 8-bit")); return; }
	if(format & WAVE_FORMAT_48S08) { _tcscpy_s(outString,noBuffElements,_T("48 kHz, Stereo, 8-bit")); return; }

	if(format & WAVE_FORMAT_96M08) { _tcscpy_s(outString,noBuffElements,_T("96 kHz, Mono, 8-bit")); return; }
	if(format & WAVE_FORMAT_96S08) { _tcscpy_s(outString,noBuffElements,_T("96 kHz, Stereo, 16-bit")); return; }

	_tcscpy_s(outString,noBuffElements,_T("Format not supported"));
}

void AudioFormats::getFormatDetails(int format, BOOL *outStereo, int *outFrequency) {

	if(format & WAVE_FORMAT_1M08) { *outStereo = FALSE;	*outFrequency = 11025; return; }
	if(format & WAVE_FORMAT_1S08) { *outStereo = TRUE;	*outFrequency = 11025; return; }

	if(format & WAVE_FORMAT_2M08) { *outStereo = FALSE;	*outFrequency = 22050; return; }
	if(format & WAVE_FORMAT_2S08) { *outStereo = TRUE;	*outFrequency = 22050; return; }

	if(format & WAVE_FORMAT_4M08) { *outStereo = FALSE;	*outFrequency = 44100; return; }
	if(format & WAVE_FORMAT_4S08) { *outStereo = TRUE;	*outFrequency = 44100; return; }

	if(format & WAVE_FORMAT_48M08) { *outStereo = FALSE;*outFrequency = 48000; return; }
	if(format & WAVE_FORMAT_48S08) { *outStereo = TRUE;	*outFrequency = 48000; return; }

	if(format & WAVE_FORMAT_96M08) { *outStereo = FALSE;*outFrequency = 96000; return; }
	if(format & WAVE_FORMAT_96S08) { *outStereo = TRUE;	*outFrequency = 96000; return; }
}

int AudioFormats::getAudioIndex(TCHAR *audioDeviceName) {

	//===================
	UINT numberOfDevices;
	//===================

	numberOfDevices = waveInGetNumDevs();

	for(UINT i=0; i<numberOfDevices; i++) {

		//==============
		WAVEINCAPS caps;
		//==============

		waveInGetDevCaps(i,&caps,sizeof(caps));

		if(! _tcscmp(caps.szPname,audioDeviceName) ) return i;
	}

	return -1;	//failed somehow
}