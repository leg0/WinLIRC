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

unsigned int ServerThread(void *srv) {((Cserver *)srv)->ThreadProc();return 0;}

Cserver::Cserver()
{
	for(int i=0;i<MAX_CLIENTS;i++) 
		clients[i]=INVALID_SOCKET;
	server=INVALID_SOCKET;
}

Cserver::~Cserver()
{
	KillThread(&ServerThreadHandle,&ServerThreadEvent);
	for(int i=0;i<MAX_CLIENTS;i++)
	{
		if(clients[i]!=INVALID_SOCKET)
		{
			DEBUG("closing socket %d\n",i);
			closesocket(clients[i]);
		}
	}
	if(server!=INVALID_SOCKET)
	{
		closesocket(server);
		DEBUG("Server socket closed.\n");
	}
	WSACleanup();
}

bool Cserver::init()
{
	/* init winsock */
	WSADATA data;
	int res=WSAStartup(0x0002,&data);
	if(res!=0) 
	{ 
		DEBUG("WSAStartup failed\n");
		MessageBox(NULL,"Could not initialize Windows Sockets.\n"
			"Note that this program requires WinSock 2.0 or higher.","WinLIRC",MB_OK);
		return false;
	}

	/* begin password stuff */
	CRegKey key;
	bool haveKey=true;
	if(key.Open(HKEY_CURRENT_USER,"Software\\LIRC")!=ERROR_SUCCESS) //First try HKCU, then HKLM
		if(key.Open(HKEY_LOCAL_MACHINE,"Software\\LIRC")
		   !=ERROR_SUCCESS)
		{
			haveKey=false;
			DEBUG("warning: can't open Software\\LIRC key");
		}

	DWORD x;
	if(!haveKey || key.QueryDWORDValue("tcp_port",x)!=ERROR_SUCCESS)
		tcp_port=IR_PORT;
	else
		tcp_port=x;

	if(startserver()==false)
		return false;

	/* start thread */
	/* THREAD_PRIORITY_IDLE combined with the REALTIME_PRIORITY_CLASS */
	/* of this program still results in a really high priority. (16 out of 31) */
	if((ServerThreadHandle=
		AfxBeginThread(ServerThread,(void *)this,THREAD_PRIORITY_IDLE))==NULL)
	{
		DEBUG("AfxBeginThread failed\n");
		return false;
	}	
    
	return true;
}

bool Cserver::startserver(void)
{
	//===========================
	struct sockaddr_in serv_addr;
	//===========================

	/* make the server socket */

	if(server!=NULL && server!=INVALID_SOCKET)
		closesocket(server);

	server = socket(AF_INET,SOCK_STREAM,0);

	if(server==INVALID_SOCKET)
	{ 
		DEBUG("socket failed, WSAGetLastError=%d\n",WSAGetLastError());
		return false;
	}

	memset(&serv_addr,0,sizeof(struct sockaddr_in));

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	serv_addr.sin_port			= htons(tcp_port);

	if(config.localConnectionsOnly) {
		serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	}

	if(bind(server,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==SOCKET_ERROR)
		{ DEBUG("bind failed\n"); return false; }

	if(listen(server,LISTENQ)==SOCKET_ERROR)
		{ DEBUG("listen failed\n"); return false; }

	return true;
}

void Cserver::stopserver(void)
{
	/* make the server socket */
	if(server!=NULL)
		closesocket(server);
	server=NULL;
}

void Cserver::send(const char *s)
{
	int i;

	for(i=0;i<MAX_CLIENTS;i++)
		if(clients[i]!=INVALID_SOCKET)
			::send(clients[i],s,strlen(s),0);
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

	if (clients[client]!=INVALID_SOCKET) {

		//==================
		int	length;
		int	sent;
		const char *pPacket;
		//==================

		length	= packet.GetLength();
		sent	= 0;
		pPacket	= packet;

		while(length>0) {
			sent = ::send(clients[client],pPacket+sent,length,0);

			if(sent == SOCKET_ERROR) break;

			length -= sent;
		}
	}
	
}

void Cserver::ThreadProc(void)
{
	if(server==INVALID_SOCKET) return;
	int i;
	
	CEvent ServerEvent;
	CEvent ClientEvent[MAX_CLIENTS];
#define MAX_DATA 1024	// longest message a client can send
	char ClientData[MAX_CLIENTS][MAX_DATA];
	char toparse[MAX_DATA];
	HANDLE events[MAX_CLIENTS+2];
	bool server_running=true;
	
	WSAEventSelect(server,ServerEvent,FD_ACCEPT);

	for(i=0;i<MAX_CLIENTS;i++) ClientData[i][0]=0;
	
	for(;;)
	{		
		int count=0;
		events[count++]=ServerThreadEvent;
		if(server_running) events[count++]=ServerEvent;
		for(i=0;i<MAX_CLIENTS;i++)
			if(clients[i]!=INVALID_SOCKET)
				events[count++]=ClientEvent[i];
		
		unsigned int res=WaitForMultipleObjects(count,events,FALSE,INFINITE);
		if(res==WAIT_OBJECT_0)
		{
			DEBUG("ServerThread terminating\n");
			AfxEndThread(0);
			return;
		}
		else if(server_running && res==(WAIT_OBJECT_0+1))
		{
			for(i=0;i<MAX_CLIENTS;i++)
				if(clients[i]==INVALID_SOCKET) break;
			if(i==MAX_CLIENTS)
			{
				DEBUG("Should have at least 1 empty client, but none found\n");
				continue;
			}
			
			clients[i]=accept(server,NULL,NULL);
			if(clients[i]==INVALID_SOCKET)
			{
				DEBUG("accept() failed\n");
				continue;
			}

			WSAEventSelect(clients[i],ClientEvent[i],FD_CLOSE|FD_READ);
			ClientEvent[i].ResetEvent();
			ClientData[i][0]='\0';
			DEBUG("Client connection %d accepted\n",i);

			for(i=0;i<MAX_CLIENTS;i++)
				if(clients[i]==INVALID_SOCKET) break;
			if(i==MAX_CLIENTS)
			{
				DEBUG("Server full. Closing server socket.\n");
				stopserver();
				server_running=false;
			}
		}
		else /* client closed or data received */
		{
			count=server_running?2:1;
			for(i=0;i<MAX_CLIENTS;i++)
			{
				if(clients[i]!=INVALID_SOCKET)
				{
					if(res==(WAIT_OBJECT_0+(count++)))
					{
						/* either we got data or the connection closed */
						int curlen=strlen(ClientData[i]);
						int maxlen=MAX_DATA-curlen-1;
						int bytes=recv(	clients[i], ClientData[i]+curlen, maxlen, 0);
						if(bytes==0 || bytes==SOCKET_ERROR)
						{
							/* Connection was closed or something's screwy */
							closesocket(clients[i]);
							clients[i]=INVALID_SOCKET;
							DEBUG("Client connection %d closed\n",i);

							if(server_running==false)
							{
								DEBUG("Slot open.  Restarting server.\n");
								if(startserver()==true)
								{
									WSAEventSelect(server,ServerEvent,FD_ACCEPT);
									server_running=true;
								}
								else
								{
									DEBUG("Server failed to restart.\n");
									stopserver();
								}
							}
						}
						else /* bytes > 0, we read data */
						{
							ClientData[i][curlen+bytes]='\0';
							char *cur=ClientData[i];
							for(;;) {
								char *nl=strchr(cur,'\n');
								if(nl==NULL) {
									if(cur!=ClientData[i]) 
										memmove(ClientData[i],cur,strlen(cur)+1);
									break;
								} else {
									*nl='\0';
									// ----------------------------
									// Do something with the received line (cur)
									DEBUG("Got string: %s\n",cur);

									//==========================
									CStringA	response;
									BOOL		success;
									char		*command = NULL;
									//==========================

									strcpy_s(toparse,cur);

									if (toparse) {
										command = strtok(toparse," \t\r");
									}

									if (!command) //ignore lines with only whitespace
									{
										cur = nl + 1;
										break;
									}
									else if (_stricmp(command,"VERSION")==0)
									{
										success = parseVersion(cur,response);
									}
									else if (_stricmp(command,"LIST")==0)
									{
										success = parseListString(cur,response);
									}
									else if (_stricmp(command,"SET_TRANSMITTERS")==0) 
									{
										success = parseTransmitters(cur,response);
									}
									else if (_stricmp(command,"SEND_ONCE")==0)
									{
										success = parseSendString(cur,response);
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

BOOL Cserver::parseSendString(char *string, CStringA &response) {

	//==========================
	char	remoteName	[128];
	char	keyName		[128];
	int		repeats;
	int		result;
	BOOL	success;

	struct ir_ncode		*codes;
	struct ir_remote	*sender;
	//==========================

	remoteName[0]	= '\0';	// null terminate
	keyName[0]		= '\0';
	repeats			= 0;
	result			= 0;

	result = sscanf_s(string,"%*s %s %s %i",remoteName,sizeof(remoteName),keyName,sizeof(keyName),&repeats);

	if(result<2) {
		response.Format("DATA\n1\nremote or code missing\n");
		return FALSE;
	}

	sender = global_remotes;

	while (sender!=NULL && _stricmp(remoteName,sender->name)) {
		sender = sender->next;
	}

	if(sender==NULL) {
		response.Format("DATA\n1\nremote not found\n");
		return FALSE;	
	}

	codes = sender->codes;

	while (codes->name!=NULL && _stricmp(keyName,codes->name)) {
		codes++;	// linked list would have been easier .. 
	}

	if(codes->name==NULL) {
		response.Format("DATA\n1\ncode not found\n");
		return FALSE;
	}

	repeats = max(repeats,sender->min_repeat);
	repeats = min(repeats,10);	// sanity check

	success = ((Cwinlirc *)AfxGetApp())->dlg->driver.sendIR(sender,codes,repeats);

	if(success==FALSE) {
		response.Format("DATA\n1\nsend failed/not supported\n");
		return FALSE;	
	}

	((Cwinlirc *)AfxGetApp())->dlg->GoBlue();

	return TRUE;
}

BOOL Cserver::parseListString(char *string, CStringA &response) {

	//====================
	char	*remoteName;
	int		n;
	BOOL	success;
	struct ir_remote *all;
	//====================

	remoteName	= strtok(NULL," \t\r");
	n			= 0;
	all			= global_remotes;
	success		= TRUE;

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
	}
	else
	{
		while (all!=NULL && _stricmp(remoteName,all->name)) {
			all = all->next;
		}

		if (all) 
		{
			//========================
			struct ir_ncode *allcodes;
			//========================

			success		= TRUE;
			allcodes	= all->codes;

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
		}
		else
		{
			success = FALSE;
			response.Format("DATA\n1\n%s%s\n","unknown remote: ",remoteName);
		}
	}

	return success;
}

BOOL Cserver::parseVersion(char *string, CStringA &response) {

	//===========
	BOOL success;
	//===========

	if (strtok(NULL," \t\r")==NULL) {
		success = TRUE;
		response.Format("DATA\n1\n%s\n",id);
	}
	else {
		success = FALSE;
		response.Format("DATA\n1\nbad send packet\n");
	}

	return success;
}

BOOL Cserver::parseTransmitters(char *string, CStringA &response) {

	//=========================
	char	*transmitterNumber;
	DWORD	transmitterMask;
	int		transNumber;
	BOOL	success;
	//=========================

	transmitterNumber	= NULL;
	transmitterMask		= 0;
	success				= TRUE;

	while(transmitterNumber = strtok(NULL," \t\r")) {

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

		//============
		Cwinlirc *app;
		//============

		app = (Cwinlirc *)AfxGetApp();

		success = app->dlg->driver.setTransmitters(transmitterMask);

		if(!success) {
			response.Format("DATA\n1\nSetTransmitters failed/not supported\n");
		}
	}

	return success;
}

