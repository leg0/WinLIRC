// Messages.cpp : Defines the entry point for the console application.
//

#pragma comment(lib, "ws2_32.lib")

#include "stdafx.h"
#include <winsock2.h>	//winsock has to be declared before win.h otherwise all hell breaks loose
#include "windows.h"

#define PORT_NUMBER 8765

int printLIRCMessages() {

	//====================
    WSADATA		w;					//Winsock startup info
    SOCKADDR_IN target;				//Information about host
	INT			error;
	SOCKET		s;
	HANDLE		events[2];
	//====================
    
    error = WSAStartup (MAKEWORD(2, 2), &w);   // Fill in WSA info
     
    if (error) return FALSE;

    if (w.wVersion != MAKEWORD(2, 2)) { 

		WSACleanup ();		// wrong WinSock version! unload ws2_32.dll
		return FALSE;
    }
    
    s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);	// Create socket

    if (s == INVALID_SOCKET) {
        return FALSE;
    }

    target.sin_family		= AF_INET;				// address family Internet
    target.sin_port			= htons(PORT_NUMBER);	// set server’s port number
    target.sin_addr.s_addr	= inet_addr("127.0.0.1");				// set server’s IP
     
    //Try connecting...

    if (connect(s, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) { 
          return FALSE;
    }

	events[0] = WSACreateEvent();
	events[1] = GetStdHandle(STD_INPUT_HANDLE);

	FlushConsoleInputBuffer(events[1]);

	WSAEventSelect(s,events[0],FD_READ);

	while(1) {

		//===================
		DWORD	result;
		CHAR	buffer[1024];
		//===================

		result = WaitForMultipleObjects(2,events,FALSE,INFINITE);

		if(result==WAIT_OBJECT_0) {

			if(recv(s,buffer,sizeof(buffer)-1,0)) {
				
				//=======================
				INT64	keycode;
				UINT	repeatCount;
				CHAR	command[128];
				CHAR	remoteName[128];
				//=======================

				if(sscanf(buffer,"%I64x %x %s %s",&keycode,&repeatCount,command,remoteName)==4) {
					printf("%016I64x %x %s %s\n",keycode,repeatCount,command, remoteName);
				}

				WSAResetEvent(events[0]);
			}
		}
		else if(result==WAIT_OBJECT_0+1) {

			//=======================
			INPUT_RECORD *inputRecords;
			DWORD numberRead;
			BOOL exitLoop;
			DWORD numberOfEvents;
			//=======================

			exitLoop		= FALSE;
			numberOfEvents	= 0;
			numberRead		= 0;

			if(GetNumberOfConsoleInputEvents(events[1],&numberOfEvents) && numberOfEvents ) {

				inputRecords = new INPUT_RECORD[numberOfEvents];

				if(ReadConsoleInput(events[1],inputRecords,numberOfEvents,&numberRead) && numberRead) {

					for(DWORD i=0; i<numberRead; i++) {
						if(inputRecords[i].EventType == KEY_EVENT) {
							exitLoop = TRUE;
							break;
						}
					}
				}

				delete [] inputRecords;

			}

			if(exitLoop) {
				break;
			}
		}

	}

	WSACloseEvent(events[0]);
	closesocket(s);
	WSACleanup();

	return TRUE;

}

int _tmain(int argc, _TCHAR* argv[])
{
	printLIRCMessages();
	return 0;
}

