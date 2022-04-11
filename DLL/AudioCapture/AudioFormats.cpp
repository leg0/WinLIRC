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

#include "AudioFormats.h"
#include <Windows.h>
#include <MMSystem.h>
#include "StringFunctions.h"
#include <cstdint>

bool AudioFormats::formatSupported(int format) {

	//ignore all 16 bit formats

	//11khz sampling frequency samples at 90us. This can produce errors up to 180us,
	//too high for the default 100us error tolerence.

	//if(format & WAVE_FORMAT_1M08) return true;
	//if(format & WAVE_FORMAT_1S08) return true;

	if (format & WAVE_FORMAT_2M08) return true;
	if (format & WAVE_FORMAT_2S08) return true;

	if (format & WAVE_FORMAT_4M08) return true;
	if (format & WAVE_FORMAT_4S08) return true;

	if (format & WAVE_FORMAT_48M08) return true;
	if (format & WAVE_FORMAT_48S08) return true;

	if (format & WAVE_FORMAT_96M08) return true;
	if (format & WAVE_FORMAT_96S08) return true;

	return false;
}

void AudioFormats::getFormatString(int format, wchar_t* outString, int noBuffElements) {

	if (!noBuffElements) return;

	if (format & WAVE_FORMAT_1M08) { wcscpy_s(outString, noBuffElements, L"11.025 kHz, Mono, 8-bit"); return; }
	if (format & WAVE_FORMAT_1S08) { wcscpy_s(outString, noBuffElements, L"11.025 kHz, Stereo, 8-bit"); return; }

	if (format & WAVE_FORMAT_2M08) { wcscpy_s(outString, noBuffElements, L"22.05 kHz, Mono, 8-bit"); return; }
	if (format & WAVE_FORMAT_2S08) { wcscpy_s(outString, noBuffElements, L"22.05 kHz, Stereo, 8-bit"); return; }

	if (format & WAVE_FORMAT_4M08) { wcscpy_s(outString, noBuffElements, L"44.1 kHz, Mono, 8-bit"); return; }
	if (format & WAVE_FORMAT_4S08) { wcscpy_s(outString, noBuffElements, L"44.1 kHz, Stereo, 8-bit"); return; }

	if (format & WAVE_FORMAT_48M08) { wcscpy_s(outString, noBuffElements, L"48 kHz, Mono, 8-bit"); return; }
	if (format & WAVE_FORMAT_48S08) { wcscpy_s(outString, noBuffElements, L"48 kHz, Stereo, 8-bit"); return; }

	if (format & WAVE_FORMAT_96M08) { wcscpy_s(outString, noBuffElements, L"96 kHz, Mono, 8-bit"); return; }
	if (format & WAVE_FORMAT_96S08) { wcscpy_s(outString, noBuffElements, L"96 kHz, Stereo, 8-bit"); return; }

	wcscpy_s(outString, noBuffElements, L"Format not supported");
}

void AudioFormats::getFormatDetails(int format, bool* outStereo, int* outFrequency) {

	if (format & WAVE_FORMAT_1M08) { *outStereo = false;	*outFrequency = 11025; return; }
	if (format & WAVE_FORMAT_1S08) { *outStereo = true;	*outFrequency = 11025; return; }

	if (format & WAVE_FORMAT_2M08) { *outStereo = false;	*outFrequency = 22050; return; }
	if (format & WAVE_FORMAT_2S08) { *outStereo = true;	*outFrequency = 22050; return; }

	if (format & WAVE_FORMAT_4M08) { *outStereo = false;	*outFrequency = 44100; return; }
	if (format & WAVE_FORMAT_4S08) { *outStereo = true;	*outFrequency = 44100; return; }

	if (format & WAVE_FORMAT_48M08) { *outStereo = false; *outFrequency = 48000; return; }
	if (format & WAVE_FORMAT_48S08) { *outStereo = true;	*outFrequency = 48000; return; }

	if (format & WAVE_FORMAT_96M08) { *outStereo = false; *outFrequency = 96000; return; }
	if (format & WAVE_FORMAT_96S08) { *outStereo = true;	*outFrequency = 96000; return; }
}

int AudioFormats::getAudioIndex(wchar_t* audioDeviceName) {

	uint32_t const numberOfDevices = waveInGetNumDevs();

	for (uint32_t i = 0; i < numberOfDevices; i++) {

		WAVEINCAPSW caps;
		waveInGetDevCapsW(i, &caps, sizeof(caps));
		removeTrailingWhiteSpace(caps.szPname);

		if (!wcscmp(caps.szPname, audioDeviceName)) return i;
	}

	return -1;	//failed somehow
}