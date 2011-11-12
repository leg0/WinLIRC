/* libtcp.h
 */

struct tcp_server_conn {
  int connectionid;
  char connType;  // S=send, R=record
  char flagForDisconnect; // 0 = no, 1 = yes
  struct tcp_server_conn *nextConn;
  char recv_data[16384];
  int write_recv_data;
} ;

void startServer(void);
void init_tcp(void);
void startServer(void);
void check_tcp_connections(void);
void addToConnList(struct tcp_server_conn *t);
void closeConn(struct tcp_server_conn *c);
void check_tcp_incoming(void);
void disconnectTcpFlagged(void);
void networkWaitForResponse(int milliseconds);

extern int tcpRxListenersCount;


void addToConnList(struct tcp_server_conn *t);

void addTcpRxData(unsigned char * sendDat, int bytes, unsigned char listenerTypeChar);

