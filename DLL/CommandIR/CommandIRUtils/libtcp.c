/*  libtcp.c
    TCP pipe interface, supporting multiple senders and receivers
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(WIN) || defined(__APPLE__)
#include <winsock.h>
#define SOCK_NONBLOCK 0
#define MSG_DONTWAIT 0
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define closesocket close
#endif


#include "libcmdir.h"
#include "libcmdir_send.h"
#include "libcmdir_record.h"
#include "libtcp.h"

#define LISTEN_PORT 65817



int tcpRxListenersCount = 0;
int tcpTimingListenersCount = 0;

// struct tcpListener * firstTcpListener;  //  = NULL ?

struct tcp_server_conn * root_tcp_server_conn;

/*
struct tcpListener {
  int sockfd;   // Socket ID
  int flagForDisconnect;
  struct tcpListener * nextListener; 
};
*/

int sockinet;

void stopServer()
{
	if (sockinet > 0) {
		closesocket(sockinet);
	}
}

  struct sockaddr_in serv_addr_in, client_addr;

void init_tcp() 
{
  startServer();

}

void startServer() 
{
//	int enable = 1;
#ifdef WIN
	u_long iMode=1;
  WSADATA wsadata;
  WSAStartup(MAKEWORD(1,1), &wsadata);
#endif

  root_tcp_server_conn = NULL;  // No connections yet to non-setup server
  
	/* create socket */
//	sockinet = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	sockinet = socket(PF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);
	if (sockinet == -1) {
		printf("Could not create TCP/IP socket\n");
		exit(-1);
	}
//	(void)setsockopt(sockinet, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	serv_addr_in.sin_family = AF_INET;
//	serv_addr_in.sin_addr.s_addr = htonl(INADDR_ANY); // address;
  serv_addr_in.sin_addr.s_addr = INADDR_ANY; 
	serv_addr_in.sin_port = htons (65123); 

	if (bind(sockinet, (struct sockaddr *)&serv_addr_in, sizeof(serv_addr_in)) == -1) {
		fprintf(stderr, "Could not assign address to socket\n");
		exit(0);
	}

#ifdef WIN
	ioctlsocket(sockinet,FIONBIO,&iMode);
#endif
	
	listen(sockinet, 3);
//	nolinger(sockinet);   // Non-blocking?
}

/* How do we want to handle multiple connections?  We have to know if they are
   send or record connections */

void check_tcp_connections() 
{
  int sin_size;
  int connected;
  struct tcp_server_conn *a;
  sin_size = sizeof(client_addr);

//  if(connected > 0) return; // ALWAYS checking for more tcp connections now

  /* No blocking; we have to handle CommandIR add/remove */
  connected = accept(
    sockinet, 
    (struct sockaddr *)&client_addr,
    &sin_size);
  
  if(connected > 0) {
    printf("New connection from (%s , %d)\n",
           inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
    a = malloc(sizeof(struct tcp_server_conn));
    a->connectionid = connected;
    a->connType = '\0'; // Not determed yet
    a->nextConn = NULL;
    a->write_recv_data = 0; // initialize
    addToConnList(a);
  }
}

void addToConnList(struct tcp_server_conn *t)
{
  struct tcp_server_conn *p;
  printf("addToConnList %p\n", t);
  
  if(root_tcp_server_conn == NULL)
  {
    root_tcp_server_conn = t;
    return;
  }
  
  for(p = root_tcp_server_conn; p != NULL; p = p->nextConn)
  {
    if(p->nextConn == NULL)
    {
      // printf("set p(%p)->nextConn = %p\n", p, t);
      p->nextConn = t;
      return;
    }
  }
  printf("Should never end up here\n");
  exit(0);
}

void closeConn(struct tcp_server_conn *c) {
  struct tcp_server_conn *p;
  
  printf("closeConn %p\n", c);
  
  closesocket(c->connectionid);
  if(c->connType == 'R')
    tcpRxListenersCount--;
  if(c->connType == 'T')
    tcpTimingListenersCount--;
  
  // Find it in the linked list, patch up the list, and free(p)
  if(c == root_tcp_server_conn) {
    root_tcp_server_conn = c->nextConn;
    free(c);
    return;
  }

  for(p = root_tcp_server_conn; p != NULL; p = p->nextConn)
  {
    if(p->nextConn == c) {  // if previous
      p->nextConn = c->nextConn;
      free(c);
      return;
    }
  }
  printf("This connection was not found in the list?! %p\n", c);
  exit(-1);
}

void check_tcp_incoming() 
{
  int bytes_recieved, processedData, slen;
  struct tcp_server_conn *p;
  
  for(p = root_tcp_server_conn; p != NULL; p = p->nextConn)
  {
    bytes_recieved = recv(p->connectionid,&p->recv_data[p->write_recv_data],4096,MSG_DONTWAIT);
    while(bytes_recieved > 0)
    {
      printf("check_tcp_incoming: %p - bytes_recieved %d\n", p, bytes_recieved);
      
      if(bytes_recieved == -1) // -1 == WOULD_BLOCK
        break; // Non-blocking; nothing to process
      
      p->write_recv_data += bytes_recieved;
      
      // The last received byte SHOULD BE a \0... Do not add in case it spans multiple packets
      if(p->recv_data[p->write_recv_data-1] == '\0')
      {
        // printf("Zero terminated - here's everything: %s\n", recv_data);
        // Process the data
        
        if(strncmp(p->recv_data, "commandir_send ", 15)==0) {
          p->connType = 'S';
          // We might have multiple packets to send:
          processedData = 0;
          while(processedData < p->write_recv_data)
          {
            // printf("Command_Send: '%s'\n", &recv_data[processedData]);
            slen = strlen(&p->recv_data[processedData]) + 1; // send_cmd replaced ' ' with '\0'
            commandir_send_cmd(&p->recv_data[processedData]);
            processedData += slen;
          }
          p->write_recv_data = 0;  // reset the internal pointer for the next one
        }
        if(strncmp(p->recv_data, "commandir_record", 16)==0) {
          p->connType = 'R';
          tcpRxListenersCount++;

          processedData = 0;
          while(processedData < p->write_recv_data)
          {
            // printf("Command_Record: '%s'\n", &recv_data[processedData]);
            slen = strlen(&p->recv_data[processedData]) + 1; // send_cmd replaced ' ' with '\0'
            commandir_record_cmd(&p->recv_data[processedData]);
            processedData += slen;
          }
          p->write_recv_data = 0;  // reset the internal pointer for the next one
        }
        if(strncmp(p->recv_data, "commandir_timing ", 17)==0) {
          p->connType = 'T';
          tcpTimingListenersCount++;
          p->write_recv_data = 0;  // reset the internal pointer for the next one
        }
      } 
// break;  // break, if we want to service all connections fairly
      bytes_recieved = recv(p->connectionid,&p->recv_data[p->write_recv_data],4096,MSG_DONTWAIT);
    }
    if(bytes_recieved == 0 || bytes_recieved < -1) {  // -1 == would block (waiting for data)
      closeConn(p);
      return;
    }
  } // check next connection
}


void disconnectTcpFlagged() 
{
  struct tcp_server_conn * t;
  if(root_tcp_server_conn == NULL) return;
  for(t = root_tcp_server_conn; t->nextConn != NULL; t = t->nextConn)
  {
    if(t->flagForDisconnect == 1)
    {
      closeConn(t);
      disconnectTcpFlagged();
      return; // Cannot continue for loop with the t we just free()'d
    }
  }
}

// CallbackForTcpData event? - we only have ONE listener?  
void addTcpRxData(unsigned char * sendDat, int bytes, unsigned char listenerTypeChar) {
  // Copy internalPipeBuffer to EACH TCP LISTENER
  
  struct tcpListener * t;
  int r, flagged = 0;
  struct tcp_server_conn *p;
  
  /* Loop through looking for Rx connections; send to them */
  
  for(p = root_tcp_server_conn; p != NULL; p = p->nextConn)
  {

//    printf("Sending %d bytes on connected sock: %d: %s\n", bytes, p->connectionid, internalPipeBuffer);
	if(p->connType != listenerTypeChar) continue;

    r = send(p->connectionid, sendDat, bytes, 0); // what happens if other side has disconnected?
    if(r < 0) {
//      printf("Error sending (%d) - flagging for disconnect\n", r);
      p->flagForDisconnect = 1;
      flagged++;
    }
//    printf("Sent %d bytes\n", r);
  }
  
  if(flagged)
    disconnectTcpFlagged();
}


