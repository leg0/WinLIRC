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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <tchar.h>

enum SP {
	SP_POSITIVE		= 0,
	SP_NEGATIVE		= 1,
	SP_AUTOMATIC	= 2,
};

class Settings 
{

public:
	Settings();

	void	setAudioDeviceName(TCHAR *name);
	void	getAudioDeviceName(TCHAR *out);

	void	setAudioFormat(int format);
	int		getAudioFormat();

	void	setChannel(bool left);
	bool	getChannel();			//left is true

	void	setPolarity(SP p);
	SP		getPolarity();

	int 	getNoiseValue();
	void	setNoiseValue(int n);

	void	saveSettings();			// to ini file
	void	loadSettings();

private:

	//========================
	TCHAR	m_deviceName[32];			// 32 is max length
	int		m_audioFormat;
	bool	m_leftChannel;			// if mono ignore
	int		m_noiseValue;
	SP		m_polarity;
	//========================
};

#endif

