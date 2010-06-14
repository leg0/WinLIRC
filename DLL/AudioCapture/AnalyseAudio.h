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