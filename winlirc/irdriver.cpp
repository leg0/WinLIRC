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

#include "stdafx.h"
#include "irdriver.h"
#include "irconfig.h"
#include "config.h"
#include "drvdlg.h"
#include "server.h"
#include "winlirc.h"

unsigned int DaemonThread(void* drv) {
    static_cast<CIRDriver*>(drv)->DaemonThreadProc();
    return 0;
}
	
CIRDriver::CIRDriver()
{
	m_daemonThreadHandle = nullptr;
	m_daemonThreadEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

CIRDriver::~CIRDriver()
{
	unloadPlugin();

	if(m_daemonThreadEvent) {
		CloseHandle(m_daemonThreadEvent);
		m_daemonThreadEvent = nullptr;
	}

	if(m_daemonThreadHandle) {
		m_daemonThreadHandle = nullptr;
	}
}

BOOL CIRDriver::loadPlugin(std::wstring plugin) {

    //
    //make sure we have cleaned up first
    //

	std::lock_guard lock{ m_dllLock };
	unloadPlugin();

	Plugin p { plugin };
    if (p.hasValidInterface())
	{
        m_dll = std::move(p);
        m_loadedPlugin = std::move(plugin);
        return TRUE;
    }

    return FALSE;
}

void CIRDriver::unloadPlugin() {

	//
	// make sure we have cleaned up
	//
	deinit();

	// daemon thread should not be dead now.
	ASSERT(m_daemonThreadHandle == nullptr);
	m_loadedPlugin = L"";
	m_dll = Plugin{ };
}

BOOL CIRDriver::init() {

	//
	// safe to call deinit first
	//
	deinit();

	// daemon thread should be dead now.
	ASSERT(m_daemonThreadHandle == nullptr);

	if(auto pluginInit = m_dll.interface_.init) {
		if(pluginInit(reinterpret_cast<WLEventHandle>(m_daemonThreadEvent))) {

			m_daemonThreadHandle = AfxBeginThread(DaemonThread, this, THREAD_PRIORITY_IDLE);

			if(m_daemonThreadHandle) {
				return TRUE;
			}
		}
		else {
			deinit();
		}
	}

	return FALSE;
}

void CIRDriver::deinit() {

	KillThread2(&m_daemonThreadHandle,m_daemonThreadEvent);
	// daemon thread should be dead now.
	ASSERT(m_daemonThreadHandle == nullptr);

	if(auto pluginDeinit = m_dll.interface_.deinit) {
		pluginDeinit();
	}
}

int	CIRDriver::sendIR(struct ir_remote *remote,struct ir_ncode *code, int repeats) {

	std::lock_guard lock{ m_dllLock };
	if (auto pluginSendIr = m_dll.interface_.sendIR) {
		return pluginSendIr(remote, code, repeats);
	}

	return 0;
}

int	CIRDriver::decodeIR(struct ir_remote *remote, char *out, size_t out_size) {

	std::lock_guard lock{ m_dllLock };
	if(auto pluginDecodeIr = m_dll.interface_.decodeIR) {
		return pluginDecodeIr(remote,out, out_size);
	}

	return 0;
}

int	CIRDriver::setTransmitters(unsigned int transmitterMask) {

	std::lock_guard lock{ m_dllLock };
	if(auto pluginSetTransmitters = m_dll.interface_.setTransmitters) {
		return pluginSetTransmitters(transmitterMask);
	}

	return 0;
}

void CIRDriver::DaemonThreadProc(void) const {

	/* Accept client connections,		*/
	/* and watch the data buffer.		*/
	/* When data comes in, decode it	*/
	/* and send the result to clients.	*/

	//==========================
	char message[PACKET_SIZE+1];
	//==========================

	auto decodeIr = [&]() {
		std::lock_guard lock{ m_dllLock };
		std::lock_guard lock2{ CS_global_remotes };

		auto pluginDecodeIr = m_dll.interface_.decodeIR;
		ASSERT(pluginDecodeIr != nullptr);
		return pluginDecodeIr(global_remotes, message, sizeof(message));
	};

	while(WaitForSingleObject(m_daemonThreadEvent, 0) == WAIT_TIMEOUT) {

		if(decodeIr()) {

			//======================
			UINT64	keyCode;
			INT		repeat;
			CHAR	command[128];
			CHAR	remoteName[128];
			//======================

			if(config.disableRepeats) {

				if(sscanf(message,"%I64x %x",&keyCode,&repeat)==2) {
				
					if(repeat) continue;
				}
			}
			else if(config.disableFirstKeyRepeats>0) {
				
				if(sscanf(message,"%I64x %x %s %s",&keyCode,&repeat,command,remoteName)==4) {
				
					if(repeat) {

						if(repeat<=config.disableFirstKeyRepeats) continue;
						else {
							sprintf(message,"%016llx %02x %s %s\n",keyCode,repeat-config.disableFirstKeyRepeats,command,remoteName);
						}
					}
				}
			}

			app.dlg->GoGreen();
			app.server.sendToClients(message);
		}

	}
}

