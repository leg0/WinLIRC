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

#ifndef SERVER__H
#define SERVER__H

#include "drvdlg.h" // INetwrokEventHandler

class Response;

class Client
{
public:
	explicit Client(SOCKET s = INVALID_SOCKET) : socket_(s) { }
	Client(Client&& src);
	Client& operator=(Client&& src);
	Client(Client const&) = delete;
	Client& operator=(Client const& src) = delete;
	~Client();

	SOCKET release();
	bool handleInput();

private:
	void close();

	bool handleSendOnce(Response& response) const;
	bool handleList(Response& response) const;
	bool handleVersion(Response& response) const;
	bool handleSetTransmitters(Response& response) const;

private:
	SOCKET socket_;
	std::string data_;
};

class Cserver
	: public INetworkEventHandler
	, public std::enable_shared_from_this<Cserver>
{
public:

	explicit Cserver(HWND hwndEventSource);
	~Cserver();

	bool startServer		(void);
	void stopServer			(void);
	bool restartServer		(void);
	void sendToClients		(const char *s);

	virtual void onNetworkEvent(SOCKET s, WORD ev, WORD er) override;

private:
	void sendData			(SOCKET socket, const char *s);
	void reply				(const char *command,int client,bool success,const char *data);

	SOCKET		m_server;
	std::unordered_map<SOCKET, Client>	m_clients;
	int			m_tcp_port;				//tcp port for server
	int			m_winsockStart;
	HWND hwndEventSource_;
};

#endif
