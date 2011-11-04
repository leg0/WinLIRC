/*  libpipes - Initial version in Utils 0.5
    Handing multiple send & rec pipes
    Program-facing pipes
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "libpipes.h"
extern unsigned char internalPipeBuffer[4096];  // libcmdir

extern int tcpRxListenersCount; // libtcp

int openPipes[2];

void init_pipeNum(int pnum) ;
void init_pipe(char * pname, int flags, int pipeNum) ;

void init_pipes()
{
  init_pipeNum(RX_PIPE);
  init_pipeNum(TX_PIPE);
}

void init_pipeNum(int pnum) 
{
  if(openPipes[pnum] > 0)
    return; // it's open, nothing to do
  switch(pnum) 
  {
    case RX_PIPE:
      init_pipe("/tmp/commandir_rx", O_WRONLY|O_NONBLOCK, RX_PIPE);
      if(openPipes[RX_PIPE] > 0) {
        // Newly opened
        tcpRxListenersCount++;
      }
      break;
    case TX_PIPE:
      init_pipe("/tmp/commandir_cmd", O_RDONLY|O_NONBLOCK, TX_PIPE);
      break;
  }
}

/* This can be done with FD/selects, but we're checking for now */
void check_pipe_statuses() 
{
  if(openPipes[0] < 1) init_pipeNum(0);
  if(openPipes[1] < 1) init_pipeNum(1);
  
  if(openPipes[TX_PIPE] > 0)
    check_pipe_send();
}

void check_pipe_send() 
{
  // Any data waiting for us to pick it up on the incoming TX pipe?
  char incoming[16384];
  int bytesRead, x, processedBytes, processNow; 
  
  bytesRead = read( openPipes[TX_PIPE], incoming, 16384);
  if(bytesRead == 0) return;
  // Incoming TX Data; we need a header packet for remote to set CommandIR names
  // or set a TX bitmask
  
  if(bytesRead < 0) return;
  
  for(x=0; x<bytesRead; x++)
  {
    if(incoming[x] == '\n')
    {
    	incoming[x] = '\0';
    }
  }
  
  processedBytes = 0;
  while(processedBytes < bytesRead)
  {
    processNow = processedBytes;
    processedBytes+=strlen(&incoming[processedBytes]) + 1;
    if(strncmp(&incoming[processNow], "commandir_send", 14)==0)
      commandir_send_cmd(&incoming[processNow]);
    if(strncmp(&incoming[processNow], "commandir_record", 16)==0) // Just change the current RX mode
      commandir_record_cmd(&incoming[processNow]); // That's valid
    if(strncmp(&incoming[processNow], "commandir_clearmem", 18)==0)
      commandir_record_clearmem(&incoming[processNow]);  // doesn't need 'incoming' right now, but for later
  }
}

void init_pipe(char * pname, int flags, int pipeNum) 
{
  openPipes[pipeNum] = open(pname, flags);
  if(!openPipes[pipeNum]) {
    exit(-1);
  }
}

void reinit_pipe(int pipeNum) 
{
  close(openPipes[pipeNum]);
  init_pipeNum(pipeNum);
}

void pipeRxSend(int bytes) {
  int r;
  if(openPipes[RX_PIPE] > 0)
  {
    r = write(openPipes[RX_PIPE], internalPipeBuffer, bytes);
    if(r < 0) {
      close(openPipes[RX_PIPE]);
//      openPipes[RX_PIPE] = (int)NULL;
      openPipes[RX_PIPE] = 0;	// Changed from NULL for Mac compatibility
      tcpRxListenersCount--;
    } /*else {
      printf("Successfully wrote %d bytes to pipe\n", r);
    }*/
  } /* else {
    printf("Trying to send to pipe, but pipe not open\n");
  }
  */
}

void closePipes()
{
  if(openPipes[0]) close(openPipes[0]);
  if(openPipes[1]) close(openPipes[1]);
}


