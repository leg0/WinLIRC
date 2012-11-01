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

unsigned int DaemonThread(void* drv) {
    static_cast<CIRDriver*>(drv)->DaemonThreadProc();
    return 0;
}
	
CIRDriver::CIRDriver()
{
	dll.initFunction			= NULL;
	dll.deinitFunction			= NULL;
	dll.hasGuiFunction			= NULL;
	dll.loadSetupGuiFunction	= NULL;
	dll.sendFunction			= NULL;
	dll.decodeFunction			= NULL;
	dll.setTransmittersFunction	= NULL;

	dll.dllFile					= NULL;

	daemonThreadHandle		= NULL;
	daemonThreadEvent		= CreateEvent(NULL,TRUE,FALSE,NULL);
}

CIRDriver::~CIRDriver()
{
	unloadPlugin();

	if(daemonThreadEvent) {
		CloseHandle(daemonThreadEvent);
		daemonThreadEvent = NULL;
	}

	if(daemonThreadHandle) {
		delete daemonThreadHandle;
		daemonThreadHandle = NULL;
	}
}

BOOL CIRDriver::loadPlugin(CString plugin) {

	//
	//make sure we have cleaned up first
	//

	CSingleLock l(&dllLock);
	unloadPlugin();

	loadedPlugin	= plugin;
	dll.dllFile		= LoadLibrary(plugin);

	if(!dll.dllFile) return FALSE;

	dll.initFunction			= (InitFunction)			GetProcAddress(dll.dllFile,"init");
	dll.deinitFunction			= (DeinitFunction)			GetProcAddress(dll.dllFile,"deinit");
	dll.hasGuiFunction			= (HasGuiFunction)			GetProcAddress(dll.dllFile,"hasGui");
	dll.loadSetupGuiFunction	= (LoadSetupGuiFunction)	GetProcAddress(dll.dllFile,"loadSetupGui");
	dll.sendFunction			= (SendFunction)			GetProcAddress(dll.dllFile,"sendIR");
	dll.decodeFunction			= (DecodeFunction)			GetProcAddress(dll.dllFile,"decodeIR");
	dll.setTransmittersFunction	= (SetTransmittersFunction)	GetProcAddress(dll.dllFile,"setTransmitters");

	if(dll.initFunction && dll.deinitFunction && dll.hasGuiFunction && dll.loadSetupGuiFunction && dll.sendFunction && dll.decodeFunction) {
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
	ASSERT(daemonThreadHandle == NULL);

	dll.initFunction			= NULL;
	dll.deinitFunction			= NULL;
	dll.hasGuiFunction			= NULL;
	dll.loadSetupGuiFunction	= NULL;
	dll.sendFunction			= NULL;
	dll.decodeFunction			= NULL;
	dll.setTransmittersFunction	= NULL;

	if(dll.dllFile) {
		FreeLibrary(dll.dllFile);
	}

	dll.dllFile					= NULL;
}

BOOL CIRDriver::init() {

	//
	// safe to call deinit first
	//
	deinit();

	// daemon thread should be dead now.
	ASSERT(daemonThreadHandle == NULL);

	if(dll.initFunction) {
		if(dll.initFunction(daemonThreadEvent)) {

			//printf("started thread ..\n");

			daemonThreadHandle = AfxBeginThread(DaemonThread, this, THREAD_PRIORITY_IDLE);

			if(daemonThreadHandle) {
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

	KillThread2(&daemonThreadHandle,daemonThreadEvent);
	// daemon thread should be dead now.
	ASSERT(daemonThreadHandle == NULL);

	if(dll.deinitFunction) {
		dll.deinitFunction();
	}
}

int	CIRDriver::sendIR(struct ir_remote *remote,struct ir_ncode *code, int repeats) {

	CSingleLock l(&dllLock);
	if(dll.sendFunction) {
		return dll.sendFunction(remote,code,repeats);
	}

	return 0;
}

int	CIRDriver::decodeIR(struct ir_remote *remote, char *out) {

	CSingleLock l(&dllLock);
	if(dll.decodeFunction) {
		return dll.decodeFunction(remote,out);
	}

	return 0;
}

int	CIRDriver::setTransmitters(unsigned int transmitterMask) {

	CSingleLock l(&dllLock);
	if(dll.setTransmittersFunction) {
		return dll.setTransmittersFunction(transmitterMask);
	}

	return 0;
}

void CIRDriver::DaemonThreadProc(void) const {

	/* Accept client connections,		*/
	/* and watch the data buffer.		*/
	/* When data comes in, decode it	*/
	/* and send the result to clients.	*/

	//=================================
	char		message[PACKET_SIZE+1];
    Cwinlirc	*app;
	//=================================

	app = (Cwinlirc *)AfxGetApp();

	while(WaitForSingleObject(daemonThreadEvent, 0) == WAIT_TIMEOUT) {

		CSingleLock l(&dllLock);
		ASSERT(dll.decodeFunction != NULL);
		if(dll.decodeFunction(global_remotes,message)) {
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

			printf("message %s\n",message);

			app->dlg->GoGreen();
			app->server->send(message);
		}

	}
}

