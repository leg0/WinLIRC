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
	m_dll.initFunction				= nullptr;
	m_dll.deinitFunction			= nullptr;
	m_dll.hasGuiFunction			= nullptr;
	m_dll.loadSetupGuiFunction		= nullptr;
	m_dll.sendFunction				= nullptr;
	m_dll.decodeFunction			= nullptr;
	m_dll.setTransmittersFunction	= nullptr;

	m_dll.dllFile					= nullptr;

	m_daemonThreadHandle			= nullptr;
	m_daemonThreadEvent				= CreateEvent(nullptr,TRUE,FALSE,nullptr);
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

BOOL CIRDriver::loadPlugin(CString plugin) {

	//
	//make sure we have cleaned up first
	//

	CSingleLock l(&m_dllLock);
	unloadPlugin();

	m_loadedPlugin	= plugin;
	m_dll.dllFile	= LoadLibrary(plugin);

	if(!m_dll.dllFile) return FALSE;

	m_dll.initFunction				= (InitFunction)			GetProcAddress(m_dll.dllFile,"init");
	m_dll.deinitFunction			= (DeinitFunction)			GetProcAddress(m_dll.dllFile,"deinit");
	m_dll.hasGuiFunction			= (HasGuiFunction)			GetProcAddress(m_dll.dllFile,"hasGui");
	m_dll.loadSetupGuiFunction		= (LoadSetupGuiFunction)	GetProcAddress(m_dll.dllFile,"loadSetupGui");
	m_dll.sendFunction				= (SendFunction)			GetProcAddress(m_dll.dllFile,"sendIR");
	m_dll.decodeFunction			= (DecodeFunction)			GetProcAddress(m_dll.dllFile,"decodeIR");
	m_dll.setTransmittersFunction	= (SetTransmittersFunction)	GetProcAddress(m_dll.dllFile,"setTransmitters");

	if(m_dll.initFunction && m_dll.deinitFunction && m_dll.hasGuiFunction && m_dll.loadSetupGuiFunction && m_dll.sendFunction && m_dll.decodeFunction) {
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

	m_dll.initFunction				= nullptr;
	m_dll.deinitFunction			= nullptr;
	m_dll.hasGuiFunction			= nullptr;
	m_dll.loadSetupGuiFunction		= nullptr;
	m_dll.sendFunction				= nullptr;
	m_dll.decodeFunction			= nullptr;
	m_dll.setTransmittersFunction	= nullptr;

	if(m_dll.dllFile) {
		FreeLibrary(m_dll.dllFile);
	}

	m_dll.dllFile					= nullptr;
}

BOOL CIRDriver::init() {

	//
	// safe to call deinit first
	//
	deinit();

	// daemon thread should be dead now.
	ASSERT(m_daemonThreadHandle == nullptr);

	if(m_dll.initFunction) {
		if(m_dll.initFunction(m_daemonThreadEvent)) {

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

	if(m_dll.deinitFunction) {
		m_dll.deinitFunction();
	}
}

int	CIRDriver::sendIR(struct ir_remote *remote,struct ir_ncode *code, int repeats) {

	CSingleLock l(&m_dllLock);
	if(m_dll.sendFunction) {
		return m_dll.sendFunction(remote,code,repeats);
	}

	return 0;
}

int	CIRDriver::decodeIR(struct ir_remote *remote, char *out) {

	CSingleLock l(&m_dllLock);
	if(m_dll.decodeFunction) {
		return m_dll.decodeFunction(remote,out);
	}

	return 0;
}

int	CIRDriver::setTransmitters(unsigned int transmitterMask) {

	CSingleLock l(&m_dllLock);
	if(m_dll.setTransmittersFunction) {
		return m_dll.setTransmittersFunction(transmitterMask);
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

	while(WaitForSingleObject(m_daemonThreadEvent, 0) == WAIT_TIMEOUT) {

		CSingleLock l(&m_dllLock);
		ASSERT(m_dll.decodeFunction != nullptr);
		if(m_dll.decodeFunction(global_remotes,message)) {
			l.Unlock();

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

