#ifndef WINLIRC_H
#define WINLIRC_H

//
// functions
//

int init_commandir();
void deinit_commandir();
int check_commandir_rec();
void send_lirc_buffer(unsigned char *buffer, int bytes, unsigned int frequency);

//
// variables
//
extern unsigned int currentTransmitterMask;

#endif

