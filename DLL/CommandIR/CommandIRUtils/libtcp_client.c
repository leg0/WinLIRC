/* libtcp_client.c
  Functions to connect a client program to libtcp
 */


#ifdef WIN
#include <winsock.h>
#define SOCK_NONBLOCK 0
#define MSG_DONTWAIT 0
#define bzero(p, l) memset(p, 0, l)
// #define bcopy(s, t, l) memmove(t, s, l)
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#include <strings.h>
#endif

#include <errno.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 65123

/*char * networkSendName;
unsigned char networkSend = 0;
unsigned char localhostPtr[] = "127.0.0.1";
 */
 
unsigned char networkSendName[160];
unsigned char networkSend = 0;
unsigned char localhostPtr[] = "127.0.0.1";

int networkClientConnect(char * connectToName);
int networkClientRecv(unsigned char * dat, int len);
void networkClientClose();

struct sockaddr_in csSA;
struct hostent *server;
int cs;
unsigned char incomingData[65536];

void networkWaitForResponse(int milliseconds)
{
  int msCount = 0;
  int currentCount;
  char retBuf[1024];
  
  while(msCount < milliseconds)
  {
    currentCount = networkClientRecv(retBuf, 1024);
    if(currentCount > 0)
    {
      // Response probably includes a \n new line:
      printf("Response from commandird: %s", retBuf); 
      // return;  // Wait for more
    }
    if( (currentCount < -1) && (currentCount != EWOULDBLOCK) && (currentCount != EAGAIN) )
    {
      printf("Connection closed (%d)\n", currentCount);
      return;
    }
#ifdef WIN
    Sleep(5);
#else
    usleep(5000);
#endif
    msCount += 5;
  }
}

int networkClientConnect(char * connectToName)
{
  int rc; 
#ifdef WIN
  WSADATA wsadata;
  WSAStartup(MAKEWORD(1,1), &wsadata);
#endif
    
	cs = socket( PF_INET , SOCK_STREAM , 0 ) ; // 0 = TCP
	if( cs == -1 ) {
		printf( "Could not create socket\n" ) ;
		exit( -1 ) ;
	}
#ifdef WIN
  memset( &csSA, 0, sizeof(csSA) );
#else
	bzero( &csSA , sizeof csSA ) ;
#endif
	csSA.sin_family = AF_INET ;
	csSA.sin_port = htons( (unsigned short)PORT ) ;
	
	server = gethostbyname( connectToName );
	
	switch(h_errno) {
	  case HOST_NOT_FOUND:
	  case NO_ADDRESS:
	  case NO_RECOVERY:
	  case TRY_AGAIN:
  	  printf("Unable to find host %s.\n", connectToName);
  	  exit(-1);
  }
	
	memcpy( (char *) &csSA.sin_addr , server->h_addr , server->h_length );

  //Connect to the server
  printf("Connecting to commandird at %s... ", connectToName);
	rc = connect( cs , (void *) &csSA , sizeof csSA ) ;
	if( rc == -1 ) {
		printf( "Could not connect to server.\n" ) ;
		exit( -1 ) ;
	}
	
	printf("Connected.\n");
  return 1;
}

int networkClientRecv(unsigned char * dat, int len)
{
  int rc;
  rc = recv( cs , dat , len , MSG_DONTWAIT );
  // printf("Received data (%d/%d)\n", rc, len);
  return rc;
}

int networkClientSend(unsigned char * bytes, int len)
{
  // buffer for saving data into before putting into file
	int rc ;

	//Send the server the name of the file we would like to request
	rc = send( cs , bytes , len, 0 ) ;
//	printf( "Sent: %d, Request: %d\n" ,rc, len);
  return rc;
}

void networkClientClose()
{
  if(cs != 0) {	// (int)NULL -> 0 for Mac compatibility. Oct 19/11
//    printf("Closing cs: %d\n", cs);
  	closesocket(cs);
  }
}

