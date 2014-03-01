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

#include "stdafx.h"
#include "server.h"
#include "remote.h"
#include <atlbase.h> //password stuff
#include "winlirc.h"
#include "drvdlg.h"

#define LISTENQ		4		// listen queue size
#define MAX_DATA	1024	// longest message a client can send
#define MAX_CLIENTS 8


class Response
{
public:
	Response()
		: success_(false)
	{ }

	Response(Response&& src)
	{
		*this = std::move(src);
	}

	Response& operator=(Response&& src)
	{
		if (this != &src)
		{
			command_ = std::move(src.command_);
			success_ = src.success_;
			data_ = std::move(src.data_);
		}
		return *this;
	}

	Response& data(std::string s)
	{
		data_.emplace_back(std::move(s));
		return *this;
	}

	Response& success()
	{
		success_ = true;
		return *this;
	}

	Response& fail()
	{
		success_ = false;
		return *this;
	}

	Response& command(std::string command)
	{
		command_ = std::move(command);
		return *this;
	}

	std::string toString() const;

private:
	std::string command_;
	bool success_;
	std::vector<std::string> data_;
};

std::string Response::toString() const
{
	std::ostringstream o;
	o << "BEIGN\n" << command_ << "\n";
	if (success_)
		o << "SUCCESS\n";
	else
		o << "ERROR\n";

	if (!data_.empty())
	{
		o << "DATA\n" << data_.size() << "\n";
		std::for_each(data_.begin(), data_.end(), [&](std::string const& s)
		{
			o << s << "\n";
		});
	}
	o << "END\n";
	return o.str();
}

Client::Client(Client&& src)
	: socket_(src.release())
{ }

Client& Client::operator = (Client&& src)
{
	if (this != &src)
	{
		close();
		socket_ = src.release();
	}
	return *this;
}

Client::~Client()
{
	close();
}

SOCKET Client::release()
{
	SOCKET const tmp = socket_;
	socket_ = INVALID_SOCKET;
	return tmp;
}

bool Client::handleInput()
{
	char buf[1024];
	auto const bytes = recv(socket_, buf, sizeof(buf), 0);
	if (bytes <= 0)
		return false;

	for (int i = 0; i < bytes; ++i)
	{
		if (buf[i] == '\n')
		{
			// Grammar of a request:
			//
			// request     = version | list | set-transmitters | send-once
			// version     = "VERSION"
			// list        = "LIST" [remote-name [code-name]]
			// remote-name = +(any char except spaces)
			// code-name   = +(any char except spaces)
			// set-transmitters = "SET_TRANSMITTERS" xxx?
			// send-once   = "SEND_ONCE" remote-name code-name [NUMBER]

			bool success = false;
			Response response;
			if (!success)
				success = handleVersion(response);
			if (!success)
				success = handleList(response);
			if (!success)
				success = handleSetTransmitters(response);
			if (!success)
				success = handleSendOnce(response);

			if (success)
			{
				std::string const& r = response.toString();
				::send(socket_, r.c_str(), static_cast<int>(r.size()), 0);
			}

			data_.clear();
		}
		else
		{
			// If we hit MAX_DATA bytes, and haven't dealt with it, throw them
			// away. There's probably something wrong on the client side.
			if (data_.size() == MAX_DATA)
				data_.clear();

			data_.push_back(buf[i]);
		}
	}
	return true;
}

void Client::close()
{
	if (socket_ != INVALID_SOCKET)
	{
		::shutdown(socket_, SD_BOTH);
		::closesocket(socket_);
	}
}

bool Client::handleSendOnce(Response& response) const
{
	char command[128] = { 0 };
	char remoteName[128] = { 0 };
	char keyName[128] = { 0 };
	int repeats = 0;
	auto const result = sscanf_s(data_.c_str(), "%s %s %s %i", command, sizeof(command), remoteName, sizeof(remoteName), keyName, sizeof(keyName), &repeats);
	if (_stricmp(command, "SEND_ONCE") != 0)
		return false;

	Response r;
	r.fail().command("SEND_ONCE");
	if (result < 2)
	{
		response = std::move(r.data("remote or code missing"));
		return true;
	}

	CSingleLock lock(&CS_global_remotes, TRUE);
	auto const sender = get_remote_by_name(global_remotes, remoteName);
	if (sender == nullptr)
	{
		response = std::move(r.data("remote not found"));
		return true;
	}

	auto const codes = get_code_by_name(sender->codes, keyName);
	if (codes == nullptr || codes->name == nullptr)
	{
		response = std::move(r.data("code not found"));
		return true;
	}

	repeats = std::max(repeats, sender->min_repeat);
	repeats = std::min(repeats, 10);	// sanity check

	// reset toggle masks
	if (has_toggle_mask(sender))
		sender->toggle_mask_state = 0;

	if (has_toggle_bit_mask(sender))
		sender->toggle_bit_mask_state = (sender->toggle_bit_mask_state^sender->toggle_bit_mask);

	if (app.dlg->driver.sendIR(sender, codes, repeats))
	{
		response = std::move(r.success());
		app.dlg->GoBlue();
		return true;
	}
	else
	{
		response = std::move(r.data("send failed/not supported"));
		return true;
	}
}

bool Client::handleList(Response& response) const
{
	char command[32] = { 0 };
	char remoteName[256];
	char codeName[256];
	auto const sscanfRes = sscanf_s(data_.c_str(), "%s %s %s",
		command, sizeof(command),
		remoteName, sizeof(remoteName)-1,
		codeName, sizeof(codeName)-1);

	if (_stricmp(command, "LIST") != 0)
		return false;

	CSingleLock lock(&CS_global_remotes, TRUE);

	Response r;
	r.command(command);

	if (sscanfRes == 1) // no remote name, no code name
	{
		// send names of all remotes in response.
		for (auto all = global_remotes; all; all = all->next)
		{
			r.data(all->name);
		}
		response = std::move(r.success());
		return true;
	}

	// find remote name 
	ir_remote* all = global_remotes;
	if (sscanfRes >= 2)
	{
		all = get_remote_by_name(all, remoteName);
		if (!all)
		{
			response = std::move(r.fail().data(std::string("unknown remote: ") + remoteName));
			return true;
		}
	}

	if (sscanfRes == 2) // have only remote name.
	{
		auto allcodes = all->codes;
		while (allcodes->name)
		{
			r.data(allcodes->name);
			++allcodes;
		}
		response = std::move(r.success());
		return true;
	}

	if (sscanfRes == 3) // remote name and code name
	{
		auto const code = get_code_by_name(all->codes, codeName);
		if (code->name)
		{
			std::ostringstream o;
			o << std::setw(16) << std::setfill('0') << std::hex << code->code << " " << code->name;
			r.success().data(o.str());
		}
		else
			r.fail().data(std::string("unknown code: ") + codeName);
		response = std::move(r);
		return true;
	}

	assert(false);
	return false;
}

bool Client::handleVersion(Response& response) const
{
	if (_stricmp(data_.c_str(), "VERSION") == 0)
	{
		response = Response().command(data_).success().data("WinLIRC " WINLIRC_VERSIONA);
		return true;
	}
	else
		return false;
}

bool Client::handleSetTransmitters(Response& response) const
{
#if 0
	//=========================
	char	*transmitterNumber;
	DWORD	transmitterMask;
	int		transNumber;
	BOOL	success;
	//=========================

	transmitterNumber = NULL;
	transmitterMask = 0;
	success = TRUE;

	while (transmitterNumber = strtok(NULL, " \t\r")) {

		transNumber = atoi(transmitterNumber);

		if (transNumber == 0) continue;

		if (transNumber>32) {
			success = FALSE;
			response = Response().fail().data("cannot support more than 32 transmitters");
			return true;
		}

		transmitterMask |= 1 << (transNumber - 1);
	}

	if (success) {

		//============
		Cwinlirc *app;
		//============

		app = (Cwinlirc *)AfxGetApp();

		success = app->dlg->driver.setTransmitters(transmitterMask);

		if (!success) {
			response = Response().fail().data("SetTransmitters failed/not supported");
			return true;
		}
	}

	return success;
#else
	return false;
#endif
}

Cserver::Cserver(HWND hwndEventSource)
	: hwndEventSource_(hwndEventSource)
	, m_server(INVALID_SOCKET)
{
	WSADATA data;
	m_winsockStart = WSAStartup(MAKEWORD(2, 0), &data);
}

Cserver::~Cserver()
{
	stopServer();
	WSACleanup();
}

bool Cserver::restartServer() {

	stopServer();
	return startServer();
}

bool Cserver::startServer()
{
	//===========================
	struct sockaddr_in serv_addr;
	//===========================

	if(m_winsockStart!=0) { 
		MessageBox(NULL,_T("Could not initialize Windows Sockets.\n")
			_T("Note that this program requires WinSock 2.0 or higher."),_T("WinLIRC"),MB_OK);
		return false;
	}

	m_server = socket(AF_INET,SOCK_STREAM,0);

	if(m_server==INVALID_SOCKET) { 
		WL_DEBUG("socket failed, WSAGetLastError=%d\n",WSAGetLastError());
		return false;
	}

	memset(&serv_addr,0,sizeof(struct sockaddr_in));

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	serv_addr.sin_port			= htons(config.serverPort);

	if(config.localConnectionsOnly) {
		serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	}

	if(bind(m_server,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==SOCKET_ERROR){ 
		WL_DEBUG("bind failed\n"); 
		return false; 
	}

	if(listen(m_server,LISTENQ)==SOCKET_ERROR) { 
		WL_DEBUG("listen failed\n"); 
		return false; 
	}

	if (WSAAsyncSelect(m_server, hwndEventSource_, WM_NETWORKEVENT, FD_ACCEPT) == SOCKET_ERROR)
		return false;

	return true;
}

void Cserver::stopServer()
{
	m_clients.clear();
	if (m_server != INVALID_SOCKET)
		::closesocket(m_server);
}

void Cserver::sendToClients(const char *s)
{
	std::for_each(m_clients.begin(), m_clients.end(), [=](std::pair<SOCKET const, Client>& p)
	{
		sendData(p.first, s);
	});
}

void Cserver::sendData(SOCKET socket, const char *s) {

	//=========
	int	length;
	int	sent;
	//=========

	if(socket==INVALID_SOCKET || !s) {	// sanity checking
		return;				
	}

	length	= (int)strlen(s);			// must be null terminated
	sent	= 0;

	while(length>0) {
		sent = send(socket,s+sent,length,0);

		if(sent == SOCKET_ERROR) break;

		length -= sent;
	}
}

void Cserver::onNetworkEvent(SOCKET s, WORD ev, WORD er)
{
	if (ev & FD_ACCEPT)
	{
		sockaddr_in clientAddress;
		int addrLen = sizeof(clientAddress);
		auto const clientSocket = ::accept(s, reinterpret_cast<sockaddr*>(&clientAddress), &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			WL_DEBUG("accept() failed: 0x%08x\n", ::WSAGetLastError());
		}
		else if (m_clients.size() == MAX_CLIENTS)
		{
			WL_DEBUG("server is full. rejecting connection\n");
			char const errorMsg[] = "Sorry the server is full.\n";
			::send(clientSocket, errorMsg, sizeof(errorMsg)-1, 0);
			::shutdown(clientSocket, SD_BOTH);
			::closesocket(clientSocket);
		}
		else
		{
			auto const clientIp = clientAddress.sin_addr.S_un.S_un_b;
			WL_DEBUG("Client connection from %d.%d.%d.%d:%d accepted (socket=%d)\n",
				clientIp.s_b1, clientIp.s_b2, clientIp.s_b3, clientIp.s_b4,
				ntohs(clientAddress.sin_port), clientSocket);

			WSAAsyncSelect(clientSocket, hwndEventSource_, WM_NETWORKEVENT, FD_CLOSE | FD_READ);
			m_clients.insert(std::make_pair(clientSocket, Client(clientSocket)));
		}
	}
	else if (ev & FD_READ)
	{
		if (m_clients.count(s))
			m_clients.at(s).handleInput();
		else
		{
			WL_DEBUG("Got data from unknown socket %d\n", s);
		}
	}
	else if (ev & FD_CLOSE)
	{
		WL_DEBUG("Connection closed. socket=%d\n", s);
		m_clients.erase(s);
	}
}
