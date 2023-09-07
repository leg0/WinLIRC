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
 * Modifications Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#include "irdriver.h"
#include <utility>

CIRDriver::CIRDriver()
	: winlirc_api{
		.plugin_api_version = winlirc_plugin_api_version,
		.getExitEvent = [](winlirc_api const*) -> WLEventHandle { return 0; },
	}
{ }

CIRDriver::~CIRDriver()
{
	unloadPlugin();
}

BOOL CIRDriver::loadPlugin(CString plugin) {

	//
	//make sure we have cleaned up first
	//
	unloadPlugin();

	loadedPlugin	= plugin;
	dllFile			= LoadLibrary(plugin);

	if(!dllFile)
		return false;

	auto gpf = (decltype(&::getPluginInterface))GetProcAddress(dllFile, "getPluginInterface");
	if (gpf)
	{
		auto pi = gpf();
		if (pi && pi->plugin_api_version == winlirc_plugin_api_version
			&& pi->init && pi->deinit && pi->hasGui && pi->loadSetupGui && pi->sendIR && pi->decodeIR)
		{
			pluginInterface = pi;
			return true;
		}
	}

	return false;
}

void CIRDriver::unloadPlugin()
{
	deinit();

	if(dllFile) {
		FreeLibrary(dllFile);
	}

	dllFile = nullptr;
}

BOOL CIRDriver::init() const
{
	deinit();

	if(pluginInterface && pluginInterface->init) {
		if(pluginInterface->init(pluginInterface, this)) {

			return TRUE;
		}
	}

	return FALSE;
}

void CIRDriver::deinit()
{
	if(pluginInterface && pluginInterface->deinit) {
		pluginInterface->deinit(std::exchange(pluginInterface, nullptr));
	}
}

int	CIRDriver::sendIR(struct ir_remote *remote,struct ir_ncode *code, int repeats) const
{
	if(pluginInterface && pluginInterface->sendIR) {
		return pluginInterface->sendIR(pluginInterface, remote,code,repeats);
	}

	return 0;
}

int	CIRDriver::decodeIR(struct ir_remote *remote, char *out, size_t out_size) const
{
	if(pluginInterface && pluginInterface->decodeIR)
	{
		return pluginInterface->decodeIR(pluginInterface, remote,out, out_size);
	}

	return 0;
}

hardware const* CIRDriver::getHardware() const
{
	if(!pluginInterface || !pluginInterface->getHardware)
		return nullptr;

	return pluginInterface->getHardware(pluginInterface);
}
