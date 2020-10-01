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

#include <algorithm>
#include <string_view>

using namespace std::string_view_literals;

#define LISTENQ		4		// listen queue size
#define MAX_DATA	1024	// longest message a client can send

UINT ServerThread(void *srv) {((Cserver *)srv)->ThreadProc();return 0;}

Cserver::Cserver()
{
	WSADATA data;
	m_winsockStart = WSAStartup(MAKEWORD(2,0),&data);
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
		MessageBox(nullptr,_T("Could not initialize Windows Sockets.\n")
			_T("Note that this program requires WinSock 2.0 or higher."),_T("WinLIRC"),MB_OK);
		return false;
	}

	Socket server{ socket(AF_INET, SOCK_STREAM, 0) };

	if(!server) { 
		WL_DEBUG("socket failed, WSAGetLastError=%d\n",WSAGetLastError());
		return false;
	}
	memset(&serv_addr,0,sizeof(sockaddr_in));

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	serv_addr.sin_port			= htons(config.serverPort);

	if(config.localConnectionsOnly) {
		serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	}

	if(bind(server.get(),(sockaddr *)&serv_addr,sizeof(serv_addr))==SOCKET_ERROR){ 
		WL_DEBUG("bind failed\n"); 
		return false; 
	}

	if(listen(server.get(),LISTENQ)==SOCKET_ERROR) { 
		WL_DEBUG("listen failed\n"); 
		return false; 
	}

	// THREAD_PRIORITY_IDLE combined with the HIGH_PRIORITY_CLASS
	// of this program still results in a really high priority. (16 out of 31)
	if((m_serverThreadHandle = AfxBeginThread(ServerThread,this,THREAD_PRIORITY_IDLE))==nullptr) {
		WL_DEBUG("AfxBeginThread failed\n");
		return false;
	}

	m_server = std::move(server);
	return true;
}

void Cserver::stopServer()
{
	KillThread(&m_serverThreadHandle,&m_serverThreadEvent);

	for (auto& client : m_clients)
	{
		client.reset();
	}

	m_server.reset();
}

void Cserver::sendToClients(const char *s)
{
	for (auto& client : m_clients)
	{
		sendData(client, s);
	}
}

void Cserver::sendData(Socket& socket, const char* s) {

	if (!socket || !s)
		return;

	int length = (int)strlen(s);			// must be null terminated
	int sent = 0;

	while (length > 0) {
		sent = send(socket.get(), s + sent, length, 0);

		if (sent == SOCKET_ERROR) break;

		length -= sent;
	}
}

void Cserver::reply(const char *command,int client,bool success,const char *data)
{
	//==============
	CStringA packet;
	//==============
	
	packet  = "BEGIN\n";
	packet += command;

	if(success)	packet += "\nSUCCESS\n";
	else		packet += "\nERROR\n";
	if(data)	packet += data;
	
	packet += "END\n";

	sendData(m_clients[client], packet);
}

void Cserver::ThreadProc(void)
{
	if (!m_server)
		return;

	//=========================================
	int		i;
	CEvent	serverEvent;
	CEvent	clientEvent[MAX_CLIENTS];
	char	clientData[MAX_CLIENTS][MAX_DATA];
	char	toparse[MAX_DATA];
	HANDLE	events[MAX_CLIENTS+2];
	//=========================================
	
	WSAEventSelect(m_server.get(), serverEvent, FD_ACCEPT);

	for(i=0;i<MAX_CLIENTS;i++) {
		clientData[i][0] = '\0';
	}
	
	for(;;)
	{		
		int count=0;
		events[count++] = m_serverThreadEvent;	// exit event
		events[count++] = serverEvent;

		for (i = 0; i < MAX_CLIENTS; i++) {
			if (m_clients[i]) {
				events[count++] = clientEvent[i];
			}
		}
		
		DWORD res = WaitForMultipleObjects(count, events, FALSE, INFINITE);

		if(res==WAIT_OBJECT_0) 
		{
			WL_DEBUG("ServerThread terminating\n");
			return;
		}
		else if(res==(WAIT_OBJECT_0+1))
		{
			for(i=0;i<MAX_CLIENTS;i++) {
				if(!m_clients[i]) break;
			}

			Socket tempSocket{ accept(m_server.get(), nullptr, nullptr) };
			if(i==MAX_CLIENTS)
			{
				auto errorMsg = "Sorry the server is full.\n"sv;
				::send(tempSocket.get(), errorMsg.data(), errorMsg.size(), 0);
				continue;
			}
			
			if(!tempSocket)
			{
				WL_DEBUG("accept() failed\n");
				continue;
			}
			m_clients[i] = std::move(tempSocket);

			WSAEventSelect(m_clients[i].get(), clientEvent[i], FD_CLOSE | FD_READ);
			clientEvent[i].ResetEvent();
			clientData[i][0]='\0';
			WL_DEBUG("Client connection %d accepted\n",i);
		}
		else /* client closed or data received */
		{
			for(i=0;i<MAX_CLIENTS;i++)
			{
				if(m_clients[i])
				{
					if(res==(WAIT_OBJECT_0+(2+i)))
					{
						/* either we got data or the connection closed */
						int curlen	= (int)strlen(clientData[i]);
						int maxlen	= MAX_DATA-curlen-1;
						int bytes	= recv(m_clients[i].get(), clientData[i]+curlen, maxlen, 0);

						if(bytes==0 || bytes==SOCKET_ERROR)
						{
							/* Connection was closed or something's screwy */
							m_clients[i].reset();
							WL_DEBUG("Client connection %d closed\n",i);
						}
						else /* bytes > 0, we read data */
						{
							clientData[i][curlen+bytes]='\0';
							char *cur=clientData[i];
							for(;;) {
								char *nl=strchr(cur,'\n');
								if(nl==nullptr) {
									if(cur!=clientData[i]) 
										memmove(clientData[i],cur,strlen(cur)+1);
									break;
								} else {
									*nl='\0';
									// ----------------------------
									// Do something with the received line (cur)
									WL_DEBUG("Got string: %s\n",cur);

									//================
									CStringA response;
									BOOL	 success;
									char	 *command;
									//================

									strcpy_s(toparse,cur);									
									command = strtok(toparse," \t\r");	// strtok is not thread safe ..
							
									if (!command) //ignore lines with only whitespace
									{
										cur = nl + 1;
										break;
									}
									
									if (_stricmp(command,"VERSION")==0) {
										success = parseVersion(cur,response);
									}
									else if (_stricmp(command,"LIST")==0) {
										success = parseListString(cur,response);
									}
									else if (_stricmp(command,"SET_TRANSMITTERS")==0) {
										success = parseTransmitters(cur,response);
									}
									else if (_stricmp(command,"SEND_ONCE")==0) {
										success = parseSendString(cur,response);
									}
									else {
										success = FALSE;
									}
	
									reply(cur,i,success>0,response);
									cur = nl + 1;
								}
							}
						
						}

						break;
					}
				}
			}
		}
	}
}

BOOL Cserver::parseSendString(const char *string, CStringA &response) {

	//==========================
	char	remoteName	[128];
	char	keyName		[128];
	int		repeats;
	int		result;
	BOOL	success;

	struct ir_ncode		*codes;
	struct ir_remote	*sender;
	//==========================

	CSingleLock lock(&CS_global_remotes,TRUE);

	remoteName[0]	= '\0';	// null terminate
	keyName[0]		= '\0';
	repeats			= 0;
	result			= 0;

	result = sscanf_s(string,"%*s %s %s %i",remoteName,sizeof(remoteName),keyName,sizeof(keyName),&repeats);

	if(result<2) {
		response.Format("DATA\n1\nremote or code missing\n");
		return FALSE;
	}

	sender = get_remote_by_name(global_remotes,remoteName);

	if(sender==nullptr) {
		response.Format("DATA\n1\nremote not found\n");
		return FALSE;	
	}

	codes = get_code_by_name(sender->codes,keyName);

	if(codes->name==nullptr) {
		response.Format("DATA\n1\ncode not found\n");
		return FALSE;
	}

	repeats = max(repeats,sender->min_repeat);
	repeats = min(repeats,10);	// sanity check

	// reset toggle masks

	if(has_toggle_mask(sender)) {
		sender->toggle_mask_state = 0;
	}

	if(has_toggle_bit_mask(sender)) {
		sender->toggle_bit_mask_state = (sender->toggle_bit_mask_state^sender->toggle_bit_mask);
	}

	success = app.dlg->driver.sendIR(sender,codes,repeats);

	if(success==FALSE) {
		response.Format("DATA\n1\nsend failed/not supported\n");
		return FALSE;	
	}

	app.dlg->GoBlue();

	return TRUE;
}

BOOL Cserver::parseListString(const char *string, CStringA &response) {

	//====================
	char	*remoteName;
	char	*codeName;
	int		n;
	struct ir_remote *all;
	//====================

	CSingleLock lock(&CS_global_remotes,TRUE);

	remoteName	= strtok(nullptr," \t\r");
	codeName	= strtok(nullptr," \t\r");
	n			= 0;
	all			= global_remotes;

	if (!remoteName)
	{
		while(all) 
		{
			n++;
			all = all->next;
		}

		if (n!=0)
		{
			response.Format("DATA\n%d\n",n);
			all = global_remotes;

			while(all)
			{
				response += all->name;
				response += "\n";
				all = all->next;
			}
		}

		return TRUE;
	}

	// find remote name 

	if(remoteName) {

		all = get_remote_by_name(all,remoteName);

		if(!all) {
			response.Format("DATA\n1\n%s%s\n","unknown remote: ",remoteName);
			return FALSE;
		}
	}

	if(remoteName && !codeName)
	{		
		//========================
		struct ir_ncode *allcodes;
		//========================

		allcodes = all->codes;

		while (allcodes->name)
		{
			n++;
			allcodes++;
		}
		if (n!=0)
		{
			response.Format("DATA\n%d\n",n);
			allcodes = all->codes;

			while(allcodes->name)
			{
				response += allcodes->name;
				response += "\n";
				allcodes++;
			}
		}
		return TRUE;
	}

	if(remoteName && codeName) {

		//====================
		struct ir_ncode *code;
		//====================

		code = get_code_by_name(all->codes,codeName);

		if(code) {
			response.Format("DATA\n1\n%016llX %s\n",code->code,code->name);
			return TRUE;
		}
		else {
			response.Format("DATA\n1\n%s%s\n","unknown code: ",codeName);
			return FALSE;
		}
	}

	return FALSE;
}

BOOL Cserver::parseVersion(const char *string, CStringA &response) {

	//===========
	BOOL success;
	//===========

	if (strtok(nullptr," \t\r")==nullptr) {
		success = TRUE;
		USES_CONVERSION;
		response.Format("DATA\n1\n%s\n",T2A(id));
	}
	else {
		success = FALSE;
		response.Format("DATA\n1\nbad send packet\n");
	}

	return success;
}

BOOL Cserver::parseTransmitters(const char *string, CStringA &response) {

	//=========================
	char	*transmitterNumber;
	DWORD	transmitterMask;
	int		transNumber;
	BOOL	success;
	//=========================

	transmitterNumber	= nullptr;
	transmitterMask		= 0;
	success				= TRUE;

	while(transmitterNumber = strtok(nullptr," \t\r")) {

		transNumber = atoi(transmitterNumber);

		if(transNumber==0) continue;

		if(transNumber>32) {
			success	= FALSE;
			response.Format("DATA\n1\ncannot support more than 32 transmitters\n");
			break;
		}

		transmitterMask |= 1 << (transNumber-1);
	}

	if(success) {

		success = app.dlg->driver.setTransmitters(transmitterMask);

		if(!success) {
			response.Format("DATA\n1\nSetTransmitters failed/not supported\n");
		}
	}

	return success;
}

