#ifndef SEND_H
#define SEND_H

void SetTransmitPort(HANDLE hCom,unsigned type);
void send (ir_ncode *data,struct ir_remote *rem, int repeats);

#endif