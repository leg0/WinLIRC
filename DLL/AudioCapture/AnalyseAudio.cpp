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

#include "AnalyseAudio.h"
#include <Math.h>
#include <stdio.h>
#include "Globals.h"
#include "LIRCDefines.h"

//
// A simple algorithm for decoding audio
//

AnalyseAudio::AnalyseAudio(int frequency, int numberOfChannels, bool leftChannel) {

	multiplyConstant	= 1000000 / (double)frequency;
	maxCount			= (~0) / (DWORD)multiplyConstant;
	sampleCount			= 0;

	numberOfChans		= numberOfChannels ;
	this->leftChannel	= leftChannel;

	//
	//basic error checking
	//
	if(numberOfChans<1)	numberOfChans = 1;
	if(numberOfChans>2)	numberOfChans = 2;

	bufferStart		= 0;
	bufferEnd		= 0;

	dataTest();

	pulse = false;

	//
	// Required LIRC data
	//
	strcpy(device,"hw");
	strcpy(name,"audio");
	fd			= -1;
	features	= LIRC_MODE_MODE2;
	send_mode	= 0;
	rec_mode	= LIRC_MODE_MODE2;
	code_length	= 0;
	resolution	= 0;
}

void AnalyseAudio::decodeData(UCHAR *data, int bytesRecorded) {

	//==================
	UCHAR currentSample;
	//==================

	for(int i=0; i<bytesRecorded; i+=numberOfChans) {

		if(numberOfChans>1 && !leftChannel) {
			currentSample = data[i+1];
		}
		else {
			currentSample = data[i];
		}

		if(sampleCount > maxCount) {
			sampleCount = 0;
		}

		else {

			//=======
			lirc_t x;
			//=======

			if(currentSample > (128 + 16)) {

				if(!pulse) {
					//
					//changing from space to pulse so add this
					//
					x = (lirc_t)((sampleCount) * multiplyConstant);
					sampleCount = 0;

					if(x>PULSE_MASK) x = PULSE_MASK;	//clamp the value otherwise we will get false PULSES if we overflow

					setData(x);
					SetEvent(dataReadyEvent);			//signal data is ready for other thread
				}
				pulse = true;
			}
			else {

				if(pulse) {
					//
					//changing from pulse to space so add this pulse finished so add
					//
					x = (lirc_t)((sampleCount) * multiplyConstant);
					sampleCount = 0;

					if(x>PULSE_MASK) x = PULSE_MASK;	//clamp the value otherwise we will get false PULSES if we overflow
					
					setData(x|PULSE_BIT);
					SetEvent(dataReadyEvent);			//signal data is ready for other thread
				}
				pulse = false;
			}

		}

		sampleCount++;
	}
}

bool AnalyseAudio::getData(UINT *out) {

	if(!dataReady()) return false;

	*out = dataBuffer[bufferStart];

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

	//
	// Dump raw data here for decoding test
	//

	/*
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
	*/
}