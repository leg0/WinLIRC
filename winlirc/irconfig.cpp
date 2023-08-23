/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
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
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#include "stdafx.h"
#include "irconfig.h"
#include "irdriver.h"
#include <atlbase.h>
#include "config.h"
#include "remote.h"
#include "config.h"

#include <spdlog/spdlog.h>

CIRConfig::CIRConfig(std::filesystem::path iniFilePath)
	: iniFilePath{iniFilePath.wstring()}
{
}

bool CIRConfig::readConfig() {

	//========================================
	std::lock_guard lock{ CS_global_remotes };
	FILE *file;
	//========================================

	if(remoteConfig.empty() || (file=_wfopen(remoteConfig.c_str(),L"r"))==nullptr)	
		return false;

	global_remotes.reset();
	
	USES_CONVERSION;
	global_remotes = read_config(file,T2A(remoteConfig.c_str()));

	fclose(file);

	/* ??? bad input causes codes to be null, but no */
	/* error is returned from read_config. */
	for (auto sr = global_remotes.get(); sr != nullptr; sr = sr->next.get())
	{
		if (sr->codes.empty())
		{
			spdlog::debug("read_config returned remote with null codes");
			global_remotes.reset();
			return false;
		}
	}

	if (global_remotes == nullptr)
	{
		spdlog::debug("read_config returned null");
		return false;
	}

	return true;
}

bool CIRConfig::writeINIFile() const
{
	writeString(L"WinLIRC", L"RemoteConfig", remoteConfig.c_str());
	writeString(L"WinLIRC", L"Plugin",plugin.c_str());
	writeInt(L"WinLIRC", L"DisableKeyRepeats", disableRepeats);
	writeInt(L"WinLIRC", L"DisableFirstKeyRepeats", disableFirstKeyRepeats);
	writeInt(L"WinLIRC", L"LocalConnectionsOnly", localConnectionsOnly);
	writeInt(L"WinLIRC", L"ServerPort", serverPort);
	writeInt(L"WinLIRC", L"ShowTrayIcon", showTrayIcon);

	return true;
}

bool CIRConfig::readINIFile()
{
	disableRepeats			= readInt(L"WinLIRC", L"DisableKeyRepeats",FALSE);
	disableFirstKeyRepeats	= readInt(L"WinLIRC", L"DisableFirstKeyRepeats",FALSE);
	localConnectionsOnly	= readInt(L"WinLIRC", L"LocalConnectionsOnly",TRUE);
	serverPort				= readInt(L"WinLIRC", L"ServerPort",8765);
	showTrayIcon			= readInt(L"WinLIRC", L"ShowTrayIcon",TRUE);
	remoteConfig			= readString(L"WinLIRC", L"RemoteConfig");
	plugin					= readString(L"WinLIRC", L"Plugin");
	
	return true;
}

std::wstring CIRConfig::readString(
	_In_opt_z_ wchar_t const* section,
	_In_opt_z_ wchar_t const* name,
	_In_opt_z_ wchar_t const* defaultValue) const
{
	std::wstring buffer(4000, L'\0');
	auto const nChars = readString(section, name, buffer.data(), buffer.size(), defaultValue);
	buffer.resize(nChars);
	return buffer;
}

size_t CIRConfig::readString(
	_In_opt_z_ wchar_t const* section,
	_In_opt_z_ wchar_t const* name,
	_Out_writes_to_opt_(outSize, return +1) wchar_t* out,
	_In_ size_t outSize,
	_In_opt_z_ wchar_t const* defaultValue) const
{
	return GetPrivateProfileStringW(section, name, defaultValue, out, static_cast<DWORD>(outSize), iniFilePath.c_str());
}

int CIRConfig::readInt(
	_In_z_ wchar_t const* section,
	_In_z_ wchar_t const* name,
	_In_ int defaultValue) const
{
	return GetPrivateProfileIntW(section, name, defaultValue, iniFilePath.c_str());
}

void CIRConfig::writeString(
	_In_opt_z_ wchar_t const* section,
	_In_opt_z_ wchar_t const* name,
	_In_opt_z_ wchar_t const* value) const
{
	WritePrivateProfileStringW(section, name, value, iniFilePath.c_str());
}

void CIRConfig::writeInt(
	_In_opt_z_ wchar_t const* section,
	_In_opt_z_ wchar_t const* name,
	_In_ int value) const
{
	WritePrivateProfileStringW(section, name, std::to_wstring(value).c_str(), iniFilePath.c_str());
}

