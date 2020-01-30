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
 */

#pragma once

static constexpr size_t MAX_CLIENTS = 8;

class Cserver
{
public:

	Cserver();
	~Cserver();

	bool startServer		();
	void stopServer			();
	bool restartServer		();
	void sendToClients		(const char* s);
	BOOL parseSendString	(const char* string, CStringA& response);
	BOOL parseListString	(const char* string, CStringA& response);
	BOOL parseVersion		(const char* string, CStringA& response);
	BOOL parseTransmitters	(const char* string, CStringA& response);

	void ThreadProc();

private:
	void sendData			(SOCKET socket, const char *s);
	void reply				(const char *command,int client,bool success,const char *data);

	SOCKET		m_server;
	SOCKET		m_clients[MAX_CLIENTS];
	int			m_tcp_port;				//tcp port for server
	CWinThread* m_serverThreadHandle;
	CEvent		m_serverThreadEvent;
	int			m_winsockStart;
};
