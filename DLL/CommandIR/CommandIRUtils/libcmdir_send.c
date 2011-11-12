/* libcmdir_send
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libcmdir.h"
#include "libvalidate.h"

int execute_commandir_send_arg(int argc, char *argv[]) ;
extern unsigned int tx_bitmask;
extern unsigned char standalone;
extern char * send_filename;

int commandir_send_cmd(char * cmdline)
{
  char *ptrIdx[1024];
  int ptrs = 0, x;
  int arg_len;
  
  arg_len = strlen(cmdline);
  printf("Processing cmd line (%d): %s\n", arg_len, cmdline);
  
  // Process cmdline args
  // Intentionally ignoring the first arg, which is the type of command
  ptrIdx[ptrs++] = cmdline; // we ignore the first one, but have to keep it consistent with calling direct
  for(x=0;x<arg_len;x++)
  {
    if(cmdline[x]==' ') {
      // Zero terminate, instead of space seperate
      ptrIdx[ptrs++] = &cmdline[x+1];
      cmdline[x] = '\0';
    }
  }
  
  if(ptrs > 0) {
    printf("Detected %d command line args\n", ptrs);
    return execute_commandir_send_arg(ptrs, ptrIdx);
  } else {
    printf("No command line args detected! (checked %d)\n", x);
  }
  return 0;
}

int execute_commandir_send_arg(int argc, char *argv[]) 
{
  // Setup a command from the Command Line / Network Send Line
  int x, y = 0;
  int emitter_num;
  unsigned int msDelay = 200;
  char * tx_char = NULL, * ptx_char;
  struct commandir_device * pcd = NULL;
  struct sendir * tx;

  /* Set any defaults */
  set_all_bitmask(0xff); // Default to All
  
  
  /* Process Command Line Args */
  
  for(x=1; x<argc; x++)
  {
    if(argv[x][0] == '-')
    {
      switch(argv[x][1])
      {
        case 's':
          // Named CommandIR - seach for this one - sendir might need to be 
          // modified for bitmasks AND selection of CommandIRs....

          if(validate_param_s(argv[x]))
          	return -1;

          if(standalone == 1) { // Standalone commandir
            hardware_scan();
          }
          pcd = getCommandIR_By_Name(&argv[x][2]);
          if(!pcd) {
            printf("CommandIR %s not found!\n", &argv[x][2]);
            return -1;
          }
          break;
        case 'e':
          // Emitter selection. 

          // Input Validation
          if(tx_bitmask) {
            invalid_command_line_parameter_exit("-b and -e cannot both be specified");
            return -1;
          }
          
					if(validate_numeric_parameter(
						argv[x], 
						"-e argument must be a valid number, eg '3'"
						))
						return -1;
						
          set_all_bitmask(0); // clear bitmask
          emitter_num = atoi(&argv[x][2]);
          if(emitter_num < 1 | emitter_num > 2048)
            invalid_command_line_parameter_exit("Invalid emitter number");
          set_long_tx_bit( emitter_num );
          break;
/*          tx_bitmask = atoi(&argv[x][2]); 
          tx_bitmask = (0x01 << (tx_bitmask-1));
          break; */
        case 'b':
          // hex tx_bitmask
          // Input Validation
          if(tx_bitmask) {
            invalid_command_line_parameter_exit("Cannot specify both -e and -b!");
            return -1;
          }
          
					if(validate_hex_parameter(
												argv[x], 
												"-b requires a hex code, eg. -b0f"
												))
						return -1;
					
          set_all_bitmask(0); // clear any existing bitmask
          convert_set_long(&argv[x][2], strlen(&argv[x][2]));  
          break;
/*          tx_bitmask = convert_quad(&argv[x][2], strlen(&argv[x][2]));  
          break;*/
          
        case 'd':
        	// Input Validation
        	if(validate_numeric_parameter(
						argv[x], 
						"-d argument must be a valid ms delay, eg '100'"
						))
						return -1;
						
          msDelay = atoi(&argv[x][2]);
          break;
        case 'r':
          // They're giving us raw codes on the Command Line
          // Use the remaining arguments
          tx_char = malloc(32767); 
          ptx_char = tx_char;
          
          if(argc == y) {
            printf("argc: %d, y: %d\n", argc, y);
          	invalid_command_line_parameter_exit("Specify a Pronto format IR code with -r to use (see commandir_send help).");
          	return -1;
          }
          
          for(y=x+1; y<argc; y++) {
          	
          	if(validate_hex_parameter(
							argv[y], 
							"-r should be in Proto format: sequences of hex quintuples, eg 003A 0015..."
							)) {
							return -1;
						}

						if(y == x+2)	// Validate frequency specified
						{
							if(get_frequency_from_hex(argv[y]) == 0)
								return -1;
						}
						
          	if(y == x+3)	// Validate number if signals they specified
          	{
          		if(y == x+3)
          		{
          			if(argc - x - 4 - 1 == convert_quad(argv[y], strlen(argv[y])))
          			{
          				// printf("Valid code count for raw codes\n");
          			} else {
          				// Also, the gap-in-code case:
          				if(argc - x - 4 - 1 != convert_quad(argv[y], strlen(argv[y])) + 1) {	
			        			invalid_command_line_parameter_exit("Error: Number of raw codes does not match the header specified number"); // \n added
			        			return -1;
	          			}	
          			}
          		}
          	}

            strncpy(ptx_char, argv[y], strlen(argv[y]) < 4 ? strlen(argv[y]) : 4 );
            ptx_char+=strlen(argv[y]) < 4 ? strlen(argv[y]) : 4 ;
            *ptx_char++ = ' ';
          }
          break;  
      }
    }
  }



  // If it wasn't -s specified, we create a tx map using all possible CommandIRs
  if(!pcd) {
    if(standalone == 1) {
      hardware_scan();
      detect_tx_per_commandir();
			clear_order();
			hardware_setorder_by_pdata_order();
    } else {  // Not standalone, and not pcd detect - use all
//      printf("Using All CommandIRs\n");
//      exit(-1);
    }
  }


  /** Transmit codes **/
  if(tx_char == NULL)
  {
    for(x=1; x<argc; x++)
    {
      if(argv[x][0] != '-')
      {
        send_filename = argv[x];
        tx_char = load_file(argv[x]);
        if(tx_char) // if we have a string from the command line OR a file
        {
          if(pcd) {
            tx = tx_char_2_struct(tx_char);
            commandir_send_specific(tx, pcd);
          } else {  // else regular old LIRC combination mode
            transmit_tx_char(tx_char);
          }
          sprintf(errorBuffer, "Successful transmit\n\0");
          send_error(SEND_OK);
          
          // Only pause if we have another signal to transmit:
          if(x < argc - 1)
#ifdef WIN
            Sleep(msDelay);
#else
            usleep(msDelay * 1000);
#endif
        }
      }
    }
  } else { 
    if(pcd) {
      tx = tx_char_2_struct(tx_char);
      commandir_send_specific(tx, pcd);
    } else {  // else regular old LIRC combination mode
//      printf("Classic transmit mode: %s\n", tx_char);
      transmit_tx_char(tx_char);
      sprintf(errorBuffer, "Successful transmit\n\0");
      send_error(SEND_OK);
    }
    free(tx_char);
  }
  
  return 0;

}
