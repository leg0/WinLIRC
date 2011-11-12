/* libcmdir_record.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libcmdir.h"
#include "libvalidate.h"

int execute_commandir_record_arg(int argc, char *argv[]);
int commandir_record_cmd(char * cmdline);

extern unsigned char standalone;
extern struct commandir_tx_order * ordered_commandir_devices;
extern char * rec_filename;
extern int gap_size;
extern unsigned char displayOnly;
extern unsigned char pipeOnly;
extern struct commandir_device * first_commandir_device;


void commandir_record_clearmem(char * cmdline)
{
  // Do we have something internal to clear, or do we poll all CommandIRs to clear
  // their internal buffers? POLL; CommandIRs don't clear fast enough; This will
  // guarantee their point.
  // Run through the CommandIR pointers and clear memory:
  struct commandir_device * pcd;
  int read_retval;
  char discard[65];
  
  printf("Clearing CommandIR Recording buffers\n");
  
  for(
    pcd = first_commandir_device; 
    pcd; 
    pcd = pcd->next_commandir_device) {
    read_retval = 1;
    while(read_retval > 0) {
      read_retval = usb_bulk_read(
        pcd->cmdir_udev,
        0x83,
        (char *)&discard,
        64,
        5000);
    }
  }
}

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
    commandir_record_clearmem(cmdline); // Clear RX Memory as a new command is detected
    return execute_commandir_record_arg(ptrs, ptrIdx);
  }
  return 0;
}




int execute_commandir_record_arg(int argc, char *argv[]) 
{
  int x;
  int gap;
  struct commandir_device * pcd = NULL;

  // Init the CommandIR, then record each of the names provided:
  if(standalone == 1) {
    hardware_scan();
    hardware_setorder();
    if(first_commandir_device == NULL)
    {
      raise_error("No CommandIRs are detected to record from.\n\0");
      return 0;
    }
  }

  // Restore any default settings:
  addGap = 1;  
  gapOnly = 0;
  gap_size = 50000;

  for(x=1; x<argc; x++)
  {
    if(argv[x][0] == '-')
    {
      switch(argv[x][1])
      {
      /*
        case 'a':
          /* Interactively, we prompt.  'Enter' to accept. Move on to recording.
             Over the network, ?  We're only receiving Hex codes, w/o gap.
             Could wait .2s for repeat signal, then add it; or send back the
             signal w/o gap if the .2s times out. ==> Better approach.
          gap = determineGapInteractive(first_commandir_device->the_commandir_device);
          printf("Ending here - %d\n", gap);
          return -1;
          */
        case 'f':
          // Do not add Gap to Hex codes
          addGap = 0;
          printf("set addGap = 0\n");
          break;
        case 'o':
          // Only display gap (hex and regular) 
          gapOnly = 1;
          break;
        case 's':
          // Named CommandIR - seach for this one
          validate_param_s(argv[x]);
          pcd = getCommandIR_By_Name(&argv[x][2]);
          if(!pcd) {
            printf("CommandIR %s not found!\n", &argv[x][2]);
            return -1;
          }
          printf("Recording from %s.\n", &argv[x][2]);
          break;
        case 'g':
        	if(validate_numeric_parameter(
						argv[x], 
						"-g requires a valid gap in microseconds, eg '-g400000' for 40ms"
						))
						return -1;
					
          gap_size = atoi(&argv[x][2]);
          if(gap_size < 1000)
          {
            sprintf(errorBuffer, "Please specify a larger gap size: %d microseconds is too small\n\0", gap_size);
            send_error(SEND_ERROR);
            return -1;
          }
          printf("Using gap %d microseconds\n", gap_size);
          break;
        case 'd':
          // Display Only - don't save to a file
          displayOnly = 1;
          break;
        default:
          sprintf(errorBuffer, "Unknown command line argument %c\n\0", argv[x][1]);
          send_error(SEND_ERROR);
          return -1;
      }
    } else {
      if(displayOnly) {
        sprintf(errorBuffer, "Do not specify a filename if you want to display codes with -d.\n\0");
        send_error(SEND_ERROR);
        return -1;
      }
      if(pipeOnly) {
        sprintf(errorBuffer, "Do not specify a filename if you want to pipe with -p.\n\0");
        send_error(SEND_ERROR);
        return -1; 
      }
      rec_filename = argv[x];
      printf("Recording %s ...\n", rec_filename);
      if(pcd)
        commandir_rec(pcd->cmdir_udev);
      else
        commandir_rec(NULL);
    }
  }

  if(standalone > 0)
  {
    if(displayOnly | pipeOnly) {
      if(pcd)
        commandir_rec(pcd->cmdir_udev);
      else
        commandir_rec(NULL);
    }
  }
  
  return 0;
}

