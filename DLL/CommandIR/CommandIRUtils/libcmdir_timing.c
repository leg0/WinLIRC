/* libcmdir_timing.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libcmdir_timing.h"
#include "libcmdir.h"
#include "libtcp.h"

unsigned char noHeader = 0;

//int execute_commandir_timing_arg(int argc, char *argv[]);
extern unsigned char standalone;
extern struct commandir_tx_order * ordered_commandir_devices;
extern struct commandir_tx_order * first_commandir_device;
extern unsigned char displayOnly;

/* Adding explicitly for timing */
extern unsigned char noHeader;
extern int currentPCA_Frequency;
extern int tcpTimingListenersCount;

void timingEvent(unsigned char switchByte, unsigned int buffer_write)
{
  static unsigned int curCount = 0;
  static unsigned int lastFrequency;
  static unsigned char stall = 1;
  static unsigned char isFirst = 1;
  unsigned char tstring[256];
  unsigned int tstring_write = 0;
  
  if(switchByte == USB_RX_PULSE_DEMOD || switchByte == USB_RX_SPACE_DEMOD)
    lastFrequency = 0;  // These do not have frequencies
  
	switch(switchByte)
	{
  	case USB_NO_DATA_BYTE:
  	  if(noHeader == 0)
  	    tstring_write += sprintf(&tstring[tstring_write], "\n");
  	  stall = 1;
      break;
		case USB_RX_PULSE:
		case USB_RX_PULSE_DEMOD:
		  if(stall == 1)
		  {
		    // Only display this if we're using the internal receiver
		      if(curCount > 0)
		        tstring_write += sprintf(&tstring[tstring_write], "\n");
        if(noHeader == 0)
        {
          if(lastFrequency > 0)
  		      tstring_write += sprintf(&tstring[tstring_write], "\nAt %4.2fkHz:\n", (float)lastFrequency/1000);
  		    else 
  		      tstring_write += sprintf(&tstring[tstring_write], "\n");
		      tstring_write += sprintf(&tstring[tstring_write], "Pulse\tSpace\tPulse\tSpace\tPulse\tSpace\n");
        }
		    stall = 0;
		    curCount = 0;
		  }
		case USB_RX_SPACE:
		case USB_RX_SPACE_DEMOD:
/*		  if(switchByte == USB_RX_SPACE || switchByte == USB_RX_SPACE_DEMOD) {
		    tstring_write += sprintf(&tstring[tstring_write], "S");
		  } else {
		    tstring_write += sprintf(&tstring[tstring_write], "P");
		  }
		  */
		  if(noHeader == 1) {
		    if(switchByte == USB_RX_PULSE || switchByte == USB_RX_PULSE_DEMOD) {
		      tstring_write += sprintf(&tstring[tstring_write], "P");
		     } else {
		      tstring_write += sprintf(&tstring[tstring_write], "S");
		     }
		  }
//		  if(buffer_write > 500000 && (noHeader == 0)) {
	    if(isFirst == 1) {
	      isFirst = 0;
	      tstring[tstring_write++] = '\0';
	    } else {
  		  tstring_write += sprintf(&tstring[tstring_write], "%d\t", buffer_write);
      }
		  if(++curCount==6)
		  {
		    // should only occur on a space?
		    curCount = 0;
  		  tstring_write += sprintf(&tstring[tstring_write], "\n");
		  }
		  if( (noHeader == 0) && (buffer_write > 50000) )
		    stall = 1;
      break;
		case USB_RX_PULSE_DEF:
		  if(currentPCA_Frequency == 0) {
		    lastFrequency = 0;
		  } else {
		    lastFrequency = 1/( ((float)(currentPCA_Frequency))/48000000);
		  }
		  return; // Nothing to print
  }
  
  if(standalone == STANDALONE_TIMING)
  {
    printf("%s", tstring);
    fflush(stdout);
  }
  else
  {
    tstring[tstring_write++] = '\0';
//    printf("Sending %d bytes (%s) to network client (there are %d)\n", tstring_write, tstring, tcpTimingListenersCount);
    addTcpRxData(tstring, tstring_write, 'T');
  }
}


/*
int commandir_record_cmd(char * cmdline)
{
  char *ptrIdx[1024];
  int ptrs = 0, x;
  int arg_len;
  
  arg_len = strlen(cmdline);
  printf("Processing cmd line (%d): %s\n", arg_len, cmdline);
  
  // Process cmdline args
  // Intentionally ignoring the first arg, which is the type of command
  ptrIdx[ptrs++] = cmdline; // we ignore the first one, but have to keep it consistent with calling direct
  for(x=0;x<arg_len-1;x++)
  {
    if(cmdline[x]==' ') {
      // Zero terminate, instead of space seperate
      ptrIdx[ptrs++] = &cmdline[x+1];
      cmdline[x] = '\0';
    }
  }
  
  if(ptrs > 0) {
    printf("Detected %d command line args\n", ptrs);
    return execute_commandir_record_arg(ptrs, ptrIdx);
  }
  return 0;
}

*/


int execute_commandir_timing_arg(int argc, char *argv[]) 
{
  int x;
  int gap;
  struct commandir_device * pcd = NULL;

  if(standalone == STANDALONE_TIMING) {
    hardware_scan();
    hardware_setorder();
    if(first_commandir_device == NULL)
    {
      printf("No CommandIRs detected\n");
      return 0;
    }
  }
  
  for(x=1; x<argc; x++)
  {
    if(argv[x][0] == '-')
    {
      switch(argv[x][1])
      {
        case 's':
          // Named CommandIR - seach for this one
          pcd = getCommandIR_By_Name(&argv[x][2]);
          if(!pcd) {
            printf("CommandIR %s not found!\n", &argv[x][2]);
            return -1;
          }
          printf("Recording from %s.\n", &argv[x][2]);
          break;
        case 'w':
        case 'n':
          break;
        default:
          printf("Unknown command line argument %c\n", argv[x][1]);
      }
    }
  }
  
  if(pcd)
    commandir_timings(pcd->cmdir_udev);
  else
    commandir_timings(NULL);

  return 0;
}

