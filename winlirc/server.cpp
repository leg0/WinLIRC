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

#include "server.h"
#include "globals.h"

unsigned int ServerThread(void *srv) {((Cserver *)srv)->ThreadProc();return 0;}

Cserver::Cserver()
{
	for(int i=0;i<MAX_CLIENTS;i++) clients[i]=INVALID_SOCKET;
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
	/* make the server socket */
	if(server!=NULL && server!=INVALID_SOCKET)
		closesocket(server);

	server=socket(AF_INET,SOCK_STREAM,0);
	if(server==INVALID_SOCKET)
	{ 
		DEBUG("socket failed, WSAGetLastError=%d\n",WSAGetLastError());
		return false;
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(IR_PORT);
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

void Cserver::ThreadProc(void)
{
	if(server==INVALID_SOCKET) return;
	int i;
	
	CEvent ServerEvent;
	CEvent ClientEvent[MAX_CLIENTS];
	HANDLE events[MAX_CLIENTS+2];
	bool server_running=true;
	
	WSAEventSelect(server,ServerEvent,FD_ACCEPT);
	
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

			WSAEventSelect(clients[i],ClientEvent[i],FD_CLOSE);
			DEBUG("Client connection %d accepted\n",i);

			for(i=0;i<MAX_CLIENTS;i++)
				if(clients[i]==INVALID_SOCKET) break;
			if(i==MAX_CLIENTS)
			{
				DEBUG("Server full.  Closing server socket.\n");
				stopserver();
				server_running=false;
			}
		}
		else /* client closed */
		{
			count=server_running?2:1;
			for(i=0;i<MAX_CLIENTS;i++)
			{
				if(clients[i]!=INVALID_SOCKET)
				{
					if(res==(WAIT_OBJECT_0+(count++)))
					{
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

						break;
					}
				}
			}
		}
	}
}

