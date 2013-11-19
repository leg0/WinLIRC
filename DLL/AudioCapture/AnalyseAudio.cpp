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
#include "../Common/LIRCDefines.h"

#define MAX_SPACE 10000	// this should work with 90%+ of remote protocols

//
// A simple algorithm for decoding audio
//

AnalyseAudio::AnalyseAudio(int frequency, int numberOfChannels, bool leftChannel, bool invertedSignal, int noiseValue) {

	m_multiplyConstant	= 1000000 / (double)frequency;
	m_maxCount			= (~0) / (DWORD)m_multiplyConstant;
	m_sampleCount		= 0;
	m_gapInSamples		= 0;

	m_numberOfChans		= numberOfChannels;
	m_inverted			= invertedSignal;
	m_leftChannel		= leftChannel;
	m_noiseValue		= noiseValue;

	//
	//basic error checking
	//
	if(m_numberOfChans<1)	m_numberOfChans = 1;
	if(m_numberOfChans>2)	m_numberOfChans = 2;

	m_bufferStart	= 0;
	m_bufferEnd		= 0;

	m_pulse = false;
}

void AnalyseAudio::decodeData(UCHAR *data, int bytesRecorded) {

	//==================
	UCHAR currentSample;
	//==================

	for(int i=0; i<bytesRecorded; i+=m_numberOfChans) {

		//=================
		bool changeToPulse;
		//=================

		changeToPulse = false;

		if(m_numberOfChans>1 && !m_leftChannel) {
			currentSample = data[i+1];
		}
		else {
			currentSample = data[i];
		}

		if(m_sampleCount > m_maxCount) {
			m_sampleCount = 0;			// every so often we will reset
			m_pulse = false;
		}

		if(m_inverted) {
			if(currentSample < (128 - m_noiseValue)) changeToPulse = true;
		}
		else {
			if(currentSample > (128 + m_noiseValue)) changeToPulse = true;
		}

		if(changeToPulse) {

			if(!m_pulse) {
				//
				//changing from space to pulse so add this
				//
				lirc_t x = (lirc_t)(m_sampleCount * m_multiplyConstant);
				m_sampleCount = 0;

				if(x>PULSE_MASK) x = PULSE_MASK;	//clamp the value otherwise we will get false PULSES if we overflow

				setData(x);
			}
			m_pulse = true;
		}
		else {

			if(m_pulse) {
				//
				//changing from pulse to space so add this pulse finished so add
				//
				lirc_t x = (lirc_t)(m_sampleCount * m_multiplyConstant);
				m_sampleCount = 0;

				if(x>PULSE_MASK) x = PULSE_MASK;	//clamp the value otherwise we will get false PULSES if we overflow
				
				setData(x|PULSE_BIT);
			}
			m_pulse = false;
		}

		sendBuffer(!changeToPulse);	// have we got enough data to kick off decoding

		m_sampleCount++;
	}
}

// we try and buffer an entire remote signal before kicking off decoding
// if we don't and the signal overlaps 2 incoming buffers
// it'll decode immediately then be sat there waiting for the length of the buffer
// waiting for the next data, which might be too long
void AnalyseAudio::sendBuffer(bool space) {

	if(space) {
		m_gapInSamples++;

		lirc_t x = (lirc_t)(m_gapInSamples * m_multiplyConstant);

		if(x>MAX_SPACE && dataReady()) {
			SetEvent(dataReadyEvent);		// set data as ready and kick off decoding in other thread
		}
	}
	else {
		m_gapInSamples = 0;
	}
}

bool AnalyseAudio::getData(UINT *out) {

	if(!dataReady()) return false;

	*out = m_dataBuffer[m_bufferStart];

	m_bufferStart++;	//yes these will wrap around with only 8 bits, that's what we want

	return true;
}

void AnalyseAudio::setData(UINT data) {

	m_dataBuffer[m_bufferEnd] = data;
	m_bufferEnd++;
}

bool AnalyseAudio::dataReady() {

	if(m_bufferStart==m_bufferEnd) return false;
	
	return true;
}