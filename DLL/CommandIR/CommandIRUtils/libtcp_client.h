/* libtcp_client.h
  Client routines for accessing libtcp server
 */

int networkClientConnect(char * connectToName);
int networkClientRecv(unsigned char * dat, int len);
void networkClientClose(void);
int networkClientSend(unsigned char * bytes, int len);

extern unsigned char networkSendName[160];
extern unsigned char networkSend;
extern unsigned char localhostPtr[];

