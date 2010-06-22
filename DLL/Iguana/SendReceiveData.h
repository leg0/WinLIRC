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

#ifndef RECEIVEDATA_H
#define RECEIVEDATA_H

#include "LIRCDefines.h"
#include "iguanaIR.h"

#define IGUANA_TRANSMITTER1	1
#define IGUANA_TRANSMITTER2	2
#define IGUANA_TRANSMITTER3	4
#define IGUANA_TRANSMITTER4	8

class SendReceiveData
{
public:
	SendReceiveData();

	bool	init();
	void	deinit();

	bool	getData(lirc_t *out);
	bool	dataReady();
	void	waitTillDataIsReady(int maxUSecs);
	void	threadProc();
	bool	setTransmitters(UCHAR channels);
	int		send(struct ir_remote *remote, struct ir_ncode *code, int repeats);
	
private:

	void	setData(lirc_t data);
	void	killThread();
	void	receiveLoop();
	bool	daemonTransaction(unsigned char code, void *value, size_t size);

	//==========================
	lirc_t		dataBuffer[256];
	UCHAR		bufferStart;
	UCHAR		bufferEnd;
	int			recvDone;
	PIPE_PTR	connection;
	PIPE_PTR	sendConnection;
	HANDLE		threadHandle;
	int			currentCarrier;
	//==========================
};

#endif