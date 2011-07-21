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

unsigned int DaemonThread(void *drv) {((CIRDriver *)drv)->DaemonThreadProc();return 0;}
	
CIRDriver::CIRDriver()
{
	initFunction			= NULL;
	deinitFunction			= NULL;
	hasGuiFunction			= NULL;
	loadSetupGuiFunction	= NULL;
	sendFunction			= NULL;
	decodeFunction			= NULL;

	dllFile					= NULL;

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
	unloadPlugin();

	loadedPlugin	= plugin;
	dllFile			= LoadLibrary(plugin);

	if(!dllFile) return FALSE;

	initFunction			= (InitFunction)		GetProcAddress(dllFile,"init");
	deinitFunction			= (DeinitFunction)		GetProcAddress(dllFile,"deinit");
	hasGuiFunction			= (HasGuiFunction)		GetProcAddress(dllFile,"hasGui");
	loadSetupGuiFunction	= (LoadSetupGuiFunction)GetProcAddress(dllFile,"loadSetupGui");
	sendFunction			= (SendFunction)		GetProcAddress(dllFile,"sendIR");
	decodeFunction			= (DecodeFunction)		GetProcAddress(dllFile,"decodeIR");

	if(initFunction && deinitFunction && hasGuiFunction && loadSetupGuiFunction && sendFunction && decodeFunction) {
		return TRUE;
	}

	return FALSE;
}

void CIRDriver::unloadPlugin() {

	//
	// make sure we have cleaned up
	//
	deinit();

	initFunction			= NULL;
	deinitFunction			= NULL;
	hasGuiFunction			= NULL;
	loadSetupGuiFunction	= NULL;
	sendFunction			= NULL;
	decodeFunction			= NULL;

	if(dllFile) {
		FreeLibrary(dllFile);
	}

	dllFile					= NULL;
	daemonThreadHandle		= NULL;
}

BOOL CIRDriver::init() {

	//
	// safe to call deinit first
	//
	deinit();

	if(initFunction) {
		if(initFunction(daemonThreadEvent)) {

			//printf("started thread ..\n");

			daemonThreadHandle = AfxBeginThread(DaemonThread,(void *)this,THREAD_PRIORITY_IDLE);

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

	if(deinitFunction) {
		deinitFunction();
	}
}

int	CIRDriver::sendIR(struct ir_remote *remote,struct ir_ncode *code, int repeats) {

	if(sendFunction) {
		return sendFunction(remote,code,repeats);
	}

	return 0;
}

int	CIRDriver::decodeIR(struct ir_remote *remote, char *out) {

	if(decodeFunction) {
		return decodeFunction(remote,out);
	}

	return 0;
}

void CIRDriver::DaemonThreadProc(void) {

	/* Accept client connections,		*/
	/* and watch the data buffer.		*/
	/* When data comes in, decode it	*/
	/* and send the result to clients.	*/

	//=================================
	char		message[PACKET_SIZE+1];
    Cwinlirc	*app;
	//=================================

	app = (Cwinlirc *)AfxGetApp();

	for(;;) {

		if(decodeFunction(global_remotes,message)) {

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

