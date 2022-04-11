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

#include "Settings.h"
#include <winlirc/winlirc_api.h>

Settings::Settings() {

	loadSettings();
}

void Settings::setAudioDeviceName(wchar_t *name) {

	wcscpy_s(m_deviceName,32,name);
}

void Settings::getAudioDeviceName(wchar_t *out) {

	wcscpy_s(out,32,m_deviceName);
}

void Settings::setAudioFormat(int format) {

	m_audioFormat = format;
}

int Settings::getAudioFormat() {
	
	return m_audioFormat;
}

void Settings::setChannel(bool left) {

	m_leftChannel = left;
}

bool Settings::getChannel() {
	
	return m_leftChannel;
}

void Settings::setPolarity(SP p) {

	m_polarity = p;
}

SP Settings::getPolarity() {

	return m_polarity;
}

int Settings::getNoiseValue() {

	return m_noiseValue;
}

void Settings::setNoiseValue(int n) {

	m_noiseValue = n;
}

void Settings::saveSettings() {
	winlirc_settings_set_wstring(L"AudioInputPlugin",L"AudioDeviceName", m_deviceName);
	winlirc_settings_set_int(L"AudioInputPlugin",L"AudioFormat", m_audioFormat);
	winlirc_settings_set_int(L"AudioInputPlugin",L"LeftChannel", m_leftChannel);
	winlirc_settings_set_int(L"AudioInputPlugin",L"Polarity", m_polarity);
	winlirc_settings_set_int(L"AudioInputPlugin",L"NoiseValue", m_noiseValue);
}

void Settings::loadSettings() {
	winlirc_settings_get_wstring(L"AudioInputPlugin", L"AudioDeviceName", m_deviceName, 32, L"");
	m_audioFormat = winlirc_settings_get_int(L"AudioInputPlugin", L"AudioFormat", 1);
	m_leftChannel = winlirc_settings_get_int(L"AudioInputPlugin", L"LeftChannel", 1) != 0;
	m_polarity = (SP)winlirc_settings_get_int(L"AudioInputPlugin", L"Polarity", SP_AUTOMATIC);
	m_noiseValue = winlirc_settings_get_int(L"AudioInputPlugin", L"NoiseValue", 16);
}