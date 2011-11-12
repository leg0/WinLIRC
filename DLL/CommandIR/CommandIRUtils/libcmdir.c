/*  CommandIR Library
    CommandIR III
    Feb 2011, Matthew Bodkin - 0.1
    Mar 2011, Matthew Bodkin - 0.2
    Jun 2011, Matthew Bodkin - 0.3 - Support for new Persistent CommandIR Memory (Name)
    Jul 2011, Matthew Bodkin - 0.4 - Support for named pipe output
 */

#ifdef WIN32
void lirc_pipe_write( int * data );
#endif

#ifdef WIN
#include <windows.h>         // Needed for Windows Sleep()
#include <conio.h>
#endif

#include <sys/timeb.h>

#include "libcmdir.h"
#include "libtcp.h"
#include "libpipes.h"

#include "commandir_tests.h"

extern int tcpRxListenersCount; // libtcp
extern unsigned char standalone;  // All CLI-facing programas

unsigned char internalPipeBuffer[4096];


usb_dev_handle *cmdir_udev[127];
// int cmdir_num_emitters[127];	// Outdated; this is part of the pcd now

int		numCommandIRs = 0;
char	incoming_packet[1024];
char	*rec_filename = NULL;
char	*send_filename = NULL;
char	determining_gap = 0;
char	outgoingBuffer[4096];
unsigned int tx_bitmask = 0;

struct commandir_device * rx_device = NULL; // Not using this, but for later compat.

int local_buffer[1000];
int write_local_buffer = 0;
int currentPCA_Frequency = (48000000/38000);	

int max_tx = 128 * 16;  // 128 x 16 CommandIR Pro's for 2048 total emitters
unsigned char long_tx_bitmask[128]; 
int max_tx_bitmask = 127;

int gap_size = 50000;

unsigned char displayOnly = 0;
unsigned char pipeOnly = 0;
unsigned char addGap = 1;   // Must be set every time by the commandir_record command
unsigned char gapOnly = 0;

int pipePtr = 0;

struct commandir_tx_order * ordered_commandir_devices = NULL;

/* New in v0.3 */
struct commandir_device * first_commandir_device = NULL;

int makeHexString(int fileFreq, int currentPCA_Frequency, int gapMicroseconds);
void timingEvent(unsigned char switchByte, unsigned int buffer_write);

unsigned char	standalone = 1;
int				tcpRxListenersCount;

extern int tcpTimingListenersCount;

#ifndef WIN

int kbhit (void)
{
  struct timeval tv;
  fd_set rdfs;

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(&rdfs);
  FD_SET (STDIN_FILENO, &rdfs);

  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &rdfs);

}

#endif


void transmit_tx_char(unsigned char * tx_char)
{
  struct sendir * tx;
  tx = tx_char_2_struct(tx_char);
  if(tx)
  {
    tx->emitterMask = tx_bitmask;
    commandir_sendir(tx);
  }
}

// Receive a standard string on the Command Line and transmit it
// eg. 0000 006c 0022 0002 0156 00ac 0015 0016 0015 0041 0015 0016 0015 0016 0015 0041
// 0000 + Freq + # + rpt

char xtod(char c)
{
  if (c>='0' && c<='9') return c-'0';
  if (c>='A' && c<='F') return c-'A'+10;
  if (c>='a' && c<='f') return c-'a'+10;
  return c=0;
}

/* New Functions in 0.2 for long bitmask management (up to 2048 transmitters) */

// Function to Zero or 0xFF all bits (clear, or set to all)
void set_all_bitmask(unsigned char d)
{
  int x;
  for(x=0;x<=max_tx_bitmask;x++)
  {
    long_tx_bitmask[x] = d;
  }
}

void set_long_tx_bit(int bitnum)
{
  int a, b;
  if(bitnum < 1 || bitnum > 2048) return;
  bitnum--; // 1 means zero shifting
  a = bitnum / 8;
  b = bitnum % 8;
  long_tx_bitmask[a] |= 0x1 << b;
}

void set_transmitters_int(unsigned int t_enable)
{
	int x, mask = 0x01;
	set_all_bitmask(0);		// Clear
	for(x=0;x<sizeof(t_enable) * 8; x++)
	{
		if(t_enable & mask)
			set_long_tx_bit(x + 1);
		mask = mask << 1;
	}
}

void convert_set_long(char * a, int len)
{
  int x, n, c;
  for(x = len - 1; x>=0; x--)
  {
    // Convert the char, then set the bits X x
    n = convert_quad(&a[x], 1);
//    printf("Converted %c to %d where x is: %d...\n", a[x], n, x);
    // n is the HEX NUMBER; we have to set based on its bits
    c = 1;
    while(n > 0)
    {
//      printf("n is %d; first bit set is: %d, c is %d\n", n, n & 0x01, c);

      if(n & 0x01)
      {
        set_long_tx_bit( (len - x - 1) * 4 + c );
      }
      n = n >> 1;
      c++;
    }
//    set_long_tx_bit(n * (x+1));
  }
}


/* Grab a bitmask from anywhere in the 256 byte stream */
int get_bitmask(int start_bit, int num_bits)
{
  int mem_start_byte, mem_start_bit, result_bitmask = 0, x, keep_bitmask = 0, bits_done = 0, build_bitmask;
  if(start_bit <= 0) return 0;
  if(num_bits <= 0) return 0;
  if(start_bit + num_bits > 2048) return 0;
  
  mem_start_byte = start_bit / 8;
  mem_start_bit = start_bit % 8;

  while(bits_done < num_bits)
  {
    build_bitmask = long_tx_bitmask[mem_start_byte] >> (mem_start_bit - 1);
    
    if(num_bits < (8 - mem_start_bit))
    {
      // Trim off extra bytes.
      for(x=0; x<num_bits; x++)
      {
        keep_bitmask = (keep_bitmask << 1) | 0x01;
      }
      build_bitmask = build_bitmask & keep_bitmask;
    }
    result_bitmask |= build_bitmask << bits_done;
    bits_done += (9 - mem_start_bit); // that's how many we just added
    mem_start_byte++;
    mem_start_bit = 1;
  }
  return result_bitmask;
}

int convert_quad(char * a, int len)
{
  int y, d, n, p;
  p = 1;
  n = 0;
  for(y = len - 1 ; y >= 0; y--)
  {
    d = xtod(a[y]) * p;
    n += d;
    p *= 0x10;
  }
  return n;
}


usb_dev_handle * hardware_scan_for(int busNum, int devId) {
	struct usb_bus *bus;
	struct usb_device *dev;

  int bus_num;

  usb_init(); 
  
	if(!usb_find_busses()) {
	  printf("No busses found!\n");
	  return 0;
	}
	
	if(!usb_find_devices()) {
	  printf("No devices found!\n");
		return 0;
	}
	
	printf("hardware_scan_for(%d, %d)\n", busNum, devId);
	
  for (bus = usb_busses; bus; bus = bus->next)
	{
	  // Is this bus==busNum?
	  bus_num = bus->dirname[2] - '0' + (bus->dirname[1] - '0') * 10 + 
	    (bus->dirname[0] - '0') * 100;
	  if(bus_num != busNum) continue;
		for (dev = bus->devices; dev; dev = dev->next)
		{
			if ( (dev->descriptor.idVendor == USB_CMDIR_VENDOR_ID) && 
				(dev->descriptor.idProduct == 3) && (dev->devnum == devId) )
			{
			  return usb_open(dev);
	    }
	   }
	}
  return NULL;
}


void * newTxOrder(unsigned char * name) 
{
  struct commandir_tx_order * ptx;
  
  ptx = malloc(sizeof(struct commandir_tx_order));
  ptx->reservedName = name;
  // We get it's name by querying the pcd
  return ptx;
}


void reserveCommandIR_By_Name(unsigned char * name) 
{
  struct commandir_tx_order * ptx;
  // Create a linked-list of names that sets the TX order.  Didn't LIRC have something like this?
  if(ordered_commandir_devices == NULL) {
    ordered_commandir_devices = newTxOrder(name);
    return;
  }
  
  for(ptx = ordered_commandir_devices; ptx->next != NULL; ptx = ptx->next);  // find the end
    ptx->next = newTxOrder(name);

}

void connectReservedCommandIRs() 
{
  struct commandir_device * pcd;
  struct commandir_tx_order * ptx;
  unsigned char validName;
  for(pcd = first_commandir_device; pcd->next_commandir_device != NULL; pcd = pcd->next_commandir_device) 
  {
    validName = 0;
    // Does this device have a name? Is the name in the reserve list?
    if(pcd->pdata == NULL)
      pcd->pdata = getCommandIR_pdata(pcd->cmdir_udev);
    if(pcd->pdata == NULL)  // No name?  Then not the one.
    {
      pcd->flag_disconnect = 1;
      continue;  
    }
    
    // Check this name against the ptx names:
    for(ptx = ordered_commandir_devices; ptx->next != NULL; ptx = ptx->next)
    {
      if(ptx->the_commandir_device != NULL) continue;   // The name has been already assigned
      if(strncmp((char *)ptx->reservedName, (char *)((struct persistent_data_03 *)pcd->pdata)->name, strlen((char *)ptx->reservedName)) == 0)
      {
        ptx->the_commandir_device = pcd;
        validName = 1;
        break;
      } 
//        printf("Checked '%s' vs '%s' - strlen(%d)\n", ptx->reservedName, ((struct persistent_data_03 *)pcd->pdata)->name, strlen(ptx->reservedName));
    }
    if(validName == 0) {
      // This pcd's name does not match one that we're looking for
//      printf("Do not need this CommandIR: %s\n", ((struct persistent_data_03 *)pcd->pdata)->name);
      pcd->flag_disconnect = 1;
    }
  }
  
  // Did we find all the CommandIRs we were looking for?
  for(ptx = ordered_commandir_devices; ptx != NULL; ptx = ptx->next)
  {
    if(ptx->the_commandir_device == NULL)
    {
      printf("Warning, did not find a CommandIR named '%s'\n", ptx->reservedName);
    }
  }
}

int hardware_check_changes()
{
  // Flag all pcd->stillFound = 0
  // Do hardware scan; for each found CommandIR, search through pcd for busnum+devnum; stillFound = 1
  // Add any that are NOT found
  // Afterwards, search through pcd for any stillFound == 0; disconnect them
  struct commandir_device * pcd, *new_pcd;
  struct usb_bus *bus;
	struct usb_device *dev;
  int searchBus, searchDev, foundIt;
  int newlyDetected = 0;
  int disconnects = 0;
  
  if(first_commandir_device != NULL)
    for(pcd = first_commandir_device; pcd != NULL; pcd = pcd->next_commandir_device) 
    {
      pcd->stillFound = 0;
    }
  
  for (bus = usb_busses; bus; bus = bus->next)
	{
		for (dev = bus->devices; dev; dev = dev->next)
		{
			if ( (dev->descriptor.idVendor == USB_CMDIR_VENDOR_ID) && 
				(dev->descriptor.idProduct == 3) )
			{
			  searchBus =  bus->dirname[2] - '0' + (bus->dirname[1] - '0') * 10
			     + (bus->dirname[0] - '0') * 100;
			  searchDev = dev->devnum;
			  foundIt = 0;
			  if(first_commandir_device != NULL)
			  {
          for(pcd = first_commandir_device; pcd != NULL; pcd = pcd->next_commandir_device) 
          {
            if(pcd->busnum == searchBus && pcd->devnum == searchDev) {
              foundIt = 1;
              pcd->stillFound = 1;
			  pcd->flag_disconnect = 0;
              //	printf("stillFound %d:%d\n", searchBus, searchDev);
              break;
            }
          }
          if(foundIt == 0) {
            // New CommandIR! Add it
			// printf("Found new CommandIR at %d:%d\n", searchBus, searchDev);
            new_pcd = addCommandIR(dev,bus);
			if(new_pcd != NULL)
			{
				newlyDetected++;
				new_pcd->stillFound = 1; // stillFound anything new of course
				// attach it on to the end of the list (that we've just finished looking up)
				// printf("Attaching new commandir to end of list\n");
				for(pcd = first_commandir_device; pcd->next_commandir_device != NULL; pcd = pcd->next_commandir_device)
				{
				}
				pcd->next_commandir_device = new_pcd;
			}
          }
        } else {
          first_commandir_device = addCommandIR(dev,bus);
		  if(first_commandir_device)	// May not have been claimable
		  {
            newlyDetected++;
			first_commandir_device->stillFound = 1;  // stillFound anything new of course
		  }
          // printf("new commandir is first detected\n");
        }
      }
    }
  }
  
  if(first_commandir_device != NULL)
    for(pcd = first_commandir_device; pcd != NULL; pcd = pcd->next_commandir_device) 
    {
      // printf("Checking stillFound: %d\n", pcd->stillFound);
      if(pcd->stillFound == 0) 
      {
        disconnects++;
        // printf("Flagged for disconnect...\n");
        pcd->flag_disconnect = 1;
      }
    }
    
  // If we have disconnects, disconnect them; then reorder.
  if(disconnects > 0)
    disconnectUnneededCommandIRs();
  if(disconnects + newlyDetected > 0) // if any changes
  {
  	// printf("Extra: Running detect_tx_per_commandir()\n");
	  detect_tx_per_commandir();
	  hardware_setorder_by_pdata_order(); // upgrade Oct 28/2011
  //  hardware_setorder();  // ALL CommandIRs  - Don't use this one if specific CommandIRs
  }
  return disconnects;

}


void commandir_disconnect(struct commandir_device * pcd) 
{
  hardware_disconnect(pcd);
  software_disconnects();
}

void hardware_disconnect(struct commandir_device *a)
{
	// To disconnect, uninit the USB object, and set the pointer to NULL
	// The cleanup function will remove this item from the chain
	usb_release_interface(a->cmdir_udev, a->interface);
	usb_close(a->cmdir_udev);
	a->cmdir_udev = NULL;
}

void software_disconnects()
{
	struct commandir_device *previous_dev = NULL;
	struct commandir_device *next_dev;

	struct commandir_device *a;
	struct commandir_tx_order *ptx;
	struct commandir_tx_order *last_ptx;

	a = first_commandir_device;

/*	struct detected_commandir *pdc;
	struct detected_commandir *last_pdc;
 */
 
	while (a) {
		if (a->cmdir_udev == NULL) {
			last_ptx = NULL;
			for (ptx = ordered_commandir_devices; ptx; ptx = ptx->next) {
				if (ptx->the_commandir_device == a) {
					if (last_ptx == NULL) {
						ordered_commandir_devices = ptx->next;
					} else {
						last_ptx->next = ptx->next;
					}
					free(ptx);
					break;
				}
			}

			if (previous_dev == NULL) {
				if (a->next_commandir_device) {
					first_commandir_device = a->next_commandir_device;
				} else {
					first_commandir_device = NULL;
				}
			} else {
				if (a->next_commandir_device) {
					previous_dev->next_commandir_device = a->next_commandir_device;
				} else {
					previous_dev->next_commandir_device = NULL;
				}
			}
			next_dev = a->next_commandir_device;

			free(a);
			if (previous_dev) {
				if (a == rx_device) {
					rx_device = previous_dev;
				}
				a = next_dev;
				previous_dev->next_commandir_device = next_dev;
			} else {
				if (a == rx_device) {
					rx_device = first_commandir_device;
				}
				a = next_dev;
				first_commandir_device = a;
			}
			continue;
		} else {
			// If this one is empty:
			previous_dev = a;
		}
		a = a->next_commandir_device;
	}			// check all of them
	numCommandIRs--;	// It's officially removed Oct 28/2011
  if(standalone == 0)	// Only show in daemon mode
		printf("CommandIR Removed, now using %d CommandIRs\n", numCommandIRs);
}


void check_commandir_add_remove() {
  int changes;
  
  changes = usb_find_busses();
  changes += usb_find_devices();

  if(changes == 0) {
  	return;
  }
  
  // USB devices have changed; have CommandIRs been added/removed?
  if(hardware_check_changes() > 0) {  // returns how many CommandIRs were disconnected
//    printf("disconnectUnneededCommandIRs()\n");
    disconnectUnneededCommandIRs();
//    hardware_setorder_by_pdata_order();	// Added Oct 28/2011
  }
}

void disconnectUnneededCommandIRs() {
  struct commandir_device * pcd;
  for(pcd = first_commandir_device; pcd != NULL; pcd = pcd->next_commandir_device)
  {
    if(pcd->flag_disconnect == 1) {
      commandir_disconnect(pcd);
      // reorder?
      disconnectUnneededCommandIRs();
      return; // pointers interrupted; rerun
    }
  }
}


struct commandir_device * addCommandIR(struct usb_device *dev, struct usb_bus *bus) 
{
  int r;
  struct commandir_device * new_pcd;

  cmdir_udev[numCommandIRs] = usb_open(dev);
  r = usb_claim_interface(cmdir_udev[numCommandIRs], 0);
  if(r<0) 
  {
    // Is this a permission problem, or is another commandir util running?
    printf(" Unable to claim CommandIR (%d).\n", r);
    return NULL;
  }
  // note, we have to create a pcd tree (from first_commandir_device)
  // Create a new holder, and assign it to first_ if we don't have a current pcd
  new_pcd = malloc(sizeof(struct commandir_device));
  new_pcd->cmdir_udev = cmdir_udev[numCommandIRs];
#ifdef WIN
  // new_pcd->busnum = bus->location;
  new_pcd->busnum = bus->dirname[2] - '0' + (bus->dirname[1] - '0') * 10
			     + (bus->dirname[0] - '0') * 100;
#else
  new_pcd->busnum = bus->dirname[2] - '0' + (bus->dirname[1] - '0') * 10
     + (bus->dirname[0] - '0') * 100;
#endif
  new_pcd->devnum = dev->devnum;
  new_pcd->pdata = NULL;  // have not loaded pdata (yet)
  new_pcd->next_tx_signal = NULL;
  new_pcd->next_commandir_device = NULL;  // Make sure this is initialized!

  new_pcd->num_transmitters = 4; // BY DEFAULT; check pdata for override
  
  numCommandIRs++;	// Increment to next available position, which is number of cmdirs.
  if(standalone == 0)	// Only show in daemon mode
		printf("CommandIR Added, now using %d CommandIRs\n", numCommandIRs);
  return new_pcd;
        
}

// Converting this to use a linked-list of containers now for Utils v0.3
int hardware_scan()
{
  // Find the first CommandIR device:
  struct usb_bus *bus;
  struct usb_device *dev;
  struct commandir_device * current_pcd = NULL;
  struct commandir_device * new_pcd;

  usb_init(); 
  
	if(!usb_find_busses()) {
	  printf("No busses found!\n");
	  return 0;
	}
	
	if(!usb_find_devices()) {
	  printf("No devices found!\n");
		return 0;
	}
 
  for (bus = usb_busses; bus; bus = bus->next)
	{
		for (dev = bus->devices; dev; dev = dev->next)
		{
			if ( (dev->descriptor.idVendor == USB_CMDIR_VENDOR_ID) && 
				(dev->descriptor.idProduct == 3) )
			{
				new_pcd = addCommandIR(dev, bus);
				if(new_pcd != NULL)	// Might not be claim-able
				{
					if(current_pcd == NULL) 
					{
						first_commandir_device = new_pcd;
					} else {
						current_pcd->next_commandir_device = new_pcd;
					}
					current_pcd = new_pcd;
					//        numCommandIRs++;	Updated in addCommandIR now
				}
			}
	  }
	}
	if(numCommandIRs == 1)
		printf("Initiating with 1 CommandIR.\n", numCommandIRs);
	else
		printf("Initiating with %d CommandIRs.\n", numCommandIRs);
	return numCommandIRs;
}

int get_frequency_from_hex(unsigned char * freqArg)
{
  int freqArgDecimal;
  float freq;
  
  freqArgDecimal = convert_quad(freqArg, 4);
  if(freqArgDecimal == 0) {
    printf("Invalid frequency\n");
    return 0;
  }
	freq = (float)1000000/((float)freqArgDecimal*(float)0.241246);
	if(freq < 25000) {
		printf("Invalid frequency specified in hex code - %5.1fkHz < 25.0kHz minimum\n", freq/1000);
		return 0;
	}
	if(freq > 68000) {
		printf("Invalid frequency specified in hex code - %5.1fkHz > 68.0kHz maximum\n", freq/1000);
		return 0;
	}
	return (int)freq;
}

struct sendir * tx_char_2_struct(unsigned char * buf)
{
  struct commandir_3_tx_signal * ptx;
  static struct sendir retSend;
  float freq;
  unsigned char * pWrite, pulse_toggle = 1;
  unsigned char * freqArg;
  unsigned char * importArg;
  unsigned int n;
  int outgoingLen;
  
  outgoingLen = ((strlen((char *)buf) / 5) - 4) * 2 + sizeof(struct commandir_3_tx_signal);
  
/*  printf("The leng(buf)=%d, so the estimated USB is: %d\n", 
    strlen(buf), 
    outgoingLen );
 */
  
  retSend.buffer = malloc(outgoingLen + 1);
  
  freqArg = &buf[5];
  importArg = &buf[20];
  
  pWrite = (unsigned char *)&retSend.buffer[ sizeof(struct commandir_3_tx_signal) ];
  ptx = (struct commandir_3_tx_signal *)retSend.buffer;
  ptx->tx_bit_mask1 = tx_bitmask ;
  ptx->tx_bit_mask2 = tx_bitmask >> 16;
  ptx->tx_min_gap = 0xffff;
  ptx->tx_signal_count = 0;
  
  
  
//  freq = (float)1000000/((float)freqArgDecimal*0.241246);
  freq = (float)get_frequency_from_hex(freqArg);
  ptx->pulse_width = (unsigned short)(48000000/freq);
  ptx->pwm_offset = ptx->pulse_width / 2;
  
//  tx_count = sizeof(struct commandir_3_tx_signal);
  
  retSend.byteCount = sizeof(struct commandir_3_tx_signal);
  
  if(send_filename)
    printf("Transmitting '%s' at%5.1fkHz\n", send_filename, freq/1000.0);
  else
    printf("Transmitting command line args at%5.1fkHz\n", freq/1000.0);
  
  while(*importArg != 0)
  {
    n = convert_quad(importArg, 4);
    if(n == 0) break;
    *pWrite++ = n & 0xff;  // low byte
    *pWrite++ = (n >> 8) | (pulse_toggle << 7);
    retSend.byteCount += 2;
    ptx->tx_signal_count++;
//    printf("Number is %d (pulse? %d)\n", n, pulse_toggle);
    pulse_toggle = !pulse_toggle;
//    tx_count+=2;
    importArg += 5;
  }
  
  printf("Encoded %d signals.\n", ptx->tx_signal_count);
  
  return &retSend;
}

char * load_file(char * filename)
{
  FILE *in;
  int filesize;
  char * fileBuffer;
  
  in = fopen(filename, "rt");
  if(!in)
  {
    printf("File not found: %s\n", filename);
    return NULL;
  }
  fseek(in, 0, SEEK_END);
  filesize = ftell(in);
  fseek(in, 0, SEEK_SET);
  
  if(filesize > 65500)
  {
    printf("File too big\n");
    return NULL;
  }
  
  fileBuffer = malloc(filesize + 1);
  
  fgets(fileBuffer, filesize, in);

  fclose(in);

  return fileBuffer;
}


void detect_tx_per_commandir()
{
  int receive_status, tries;
  struct commandirIII_status * sptr;
  struct commandir_device * pcd;
  
  for(pcd = first_commandir_device; pcd; pcd = pcd->next_commandir_device) 

//  for(x = 0; x<numCommandIRs; x++)
  {
    tries = 10;
    while(tries-- > 0) {
    receive_status = usb_bulk_read(
      pcd->cmdir_udev,
      0x81,
      incoming_packet,
      64,
      5000);

      if (receive_status == 8) {
        // Update the commandir_device with what we just received.
        sptr = (struct commandirIII_status *)incoming_packet;
//        cmdir_num_emitters[x] = (sptr->tx_status & 0x1F) + 1;
//        printf("CommandIR %d has %d emitters.\n", x+1, cmdir_num_emitters[x]);
        pcd->num_transmitters = (sptr->tx_status & 0x1F) + 1;
//        printf("Detected that CommandIR %p has %d emitters.\n", pcd, pcd->num_transmitters);
        break;
      }// else {
//        printf("Unable to get status from CommandIR %d - we got %d bytes.\n", x+1, receive_status);
//      }
    }
  }
}


struct commandir_device * getCommandIR_By_Name(unsigned char * n) {
  struct commandir_device * pcd;
  for(pcd = first_commandir_device; pcd; pcd = pcd->next_commandir_device) {
    if(pcd->pdata == NULL)
      pcd->pdata = getCommandIR_pdata(pcd->cmdir_udev);
    if(pcd->pdata == NULL) continue;  // Not supported
    if(strcmp(n, ((struct persistent_data_03 *)pcd->pdata)->name) == 0)
      return pcd;
  }
  return NULL;
}




/* Support multiple CommandIRs */
void commandir_send_specific(struct sendir * tx, struct commandir_device * pcd)
{
  int bitmask, first_bit = 1, bytes, send_status;
  struct commandir_3_tx_signal * ptx;
  char * curPacket;
  
  bitmask = get_bitmask(first_bit, 16);
  ptx = (struct commandir_3_tx_signal * )tx->buffer;
  ptx->tx_bit_mask1 = bitmask ;
  ptx->tx_bit_mask2 = bitmask >> 16;
  
  curPacket = tx->buffer;
  bytes = tx->byteCount;
  
  while(bytes > 0)
  {
	  send_status=usb_bulk_write(
		  pcd->cmdir_udev, 
		  2,
		  curPacket,
		  bytes > 64 ? 64 : bytes,
		  50);
		if(bytes > 64)
		{
		  curPacket+=64;
		  bytes-=64;
		} else {
		  bytes = 0;
		}
		if(send_status < 0) 
		{
		  printf("CommandIR Disconnect\n");
		  // software disconnect?
		}
  }
}

void commandir_sendir(struct sendir * tx)
{
  // Convert the overall bitmask. 
  unsigned int bitmask;
  int first_bit = 1;
  struct commandir_3_tx_signal * ptx;
  struct commandir_device * pcd;
  struct commandir_tx_order * pcd_ordered;
	
  for(pcd_ordered = ordered_commandir_devices; 
		pcd_ordered != NULL; 
		pcd_ordered = pcd_ordered->next)
  {
	pcd = pcd_ordered->the_commandir_device; 	// shortcut	
    bitmask = get_bitmask(first_bit, pcd->num_transmitters);
	/* printf("commandir_sendir: device %p, num_transmitters %d, bitmask %d.\n", 
		pcd,
		pcd->num_transmitters, 
		bitmask); */
    first_bit+=pcd->num_transmitters;
    // printf("Using  bitmask %02X for CommandIR %p\n", bitmask, pcd);
    if(bitmask == 0) continue;  // nothing for this CommandIR to transmit

	/* Replacing the bitmask in place */
    ptx = (struct commandir_3_tx_signal * )tx->buffer;
    ptx->tx_bit_mask1 = (bitmask & 0x00ff);
    ptx->tx_bit_mask2 = (bitmask >> 16);
    commandir_send(tx->buffer, tx->byteCount, pcd);	// this has to be by pcd, not number!
  }
  // Return a message to say the command was a success
}


/* Send to a specific CommandIR */
int commandir_send(char * packet, int bytes, struct commandir_device * pcd)
{
  int send_status;
  char * curPacket;

  if(numCommandIRs == 0)
    return 0;
  
  curPacket = packet;
  
  while(bytes > 0)
  {
  
	  send_status=usb_bulk_write(
		  pcd->cmdir_udev, 
		  2,
		  curPacket,
		  bytes > 64 ? 64 : bytes,
		  50);
//		printf("Sent %d bytes...\n", send_status);
		if(bytes > 64)
		{
		  curPacket+=64;
		  bytes-=64;
		} else {
		  bytes = 0;
		}
//    printf(" send_status: %d\n", send_status);
  }
  return send_status;
}

/* Show CommandIR Timing information (pulse/space) for general debugging */
int commandir_timings(usb_dev_handle * udev)
{
  //  Let's do a loop and print the recorded signals in HEX format
  int read_retval;
  
  if(standalone == STANDALONE_TIMING)
    printf("Press buttons on your remote pointed at either an external receiver or the\nCommandIR recording LED.\n");
  
  if(udev == NULL)
    udev = cmdir_udev[0];
  
  // If we're a network server, we DO NOT LOOP HERE
  
#ifdef WIN32
  while(!_kbhit())
#else
  while(!kbhit())
#endif
  {
    read_retval = usb_bulk_read(
      udev,
      0x83,
      incoming_packet,
      64,
      5000);

    if(read_retval > 0)
    {
      commandir3_convert_RX(incoming_packet, read_retval -1);
    }
    
    if(standalone == 0)
      return read_retval;
#ifdef WIN
    Sleep(1);
#else
    usleep(1000);
#endif
  }

  if(!displayOnly) {
    getchar();
    printf("\n");
  }
  return 0;
}

int commandir_rec(usb_dev_handle * udev)
{
  //  Let's do a loop and print the recorded signals in HEX format
  int read_retval;
  unsigned char no_data_byte_sim[6] = {'\0', '\1', '\0', '\0', '\0', USB_NO_DATA_BYTE};
  int sim_timeout;

  struct timeb last_tp;
  struct timeb tp;
  double last_time = 0.0, now_time;

  if((displayOnly + pipeOnly) == 0)
    printf("Press the button on your remote a few times until the IR codes look 'stable'. \nPress 'enter' on the keyboard to save the most recent IR code to the file.\n");
  if(displayOnly && (standalone > 0))
    printf("Press buttons on your remote now, and press 'enter' (or Ctrl+C) on the keyboard to quit.\n");
  
  if(udev == NULL)
    udev = cmdir_udev[0];
  
  // If we're a network server, we DO NOT LOOP HERE
  
  ftime(&last_tp);
  sim_timeout = 1;
  
#ifdef WIN32
  while(!_kbhit())
#else
  while(!kbhit())
#endif
  {
    read_retval = usb_bulk_read(
      udev,
      0x83,
      incoming_packet,
      64,
      5000);

    if(read_retval > 0)
    {
      commandir3_convert_RX(incoming_packet, read_retval -1);
      ftime(&last_tp);
      last_time = last_tp.millitm / 1000.0 + last_tp.time + gap_size / 1000000.0 + 0.1;
      sim_timeout = 1;
    } else {
      if(sim_timeout == 1) {
        ftime(&tp);
        now_time = tp.millitm / 1000.0 + tp.time;
        
        if(now_time > last_time ) {
          // Simulate a NO DATA BYTE (timeout)
          // Cancel the timeout simulate; Firmwrae 3.46 puts them back in
//          commandir3_convert_RX(no_data_byte_sim, 6);
          sim_timeout = 0;
        }
      }
    }
    
    if(standalone == 0)
      return read_retval;
#ifdef WIN
    Sleep(1);
#else
    usleep(1000);
#endif
  }
  getchar();  // Eat char, or commandird will not poll anymore

  if( (!displayOnly) && (standalone > 0) ) {
    printf("Saved %s.\n", rec_filename);
  }
  return 0;
}

/*
int lirc_pipe_write(int * readyDataByte)
{
  static niceprint = 1;
  return 0;
  if(++niceprint > 6) 
  {
    niceprint = 1;
    printf("\n");
  }
  
  // So now we want to build items recognizable as specific codes; split by >50,000
  
  printf("%d  ", (0x7ff) & *readyDataByte);
  
  if(*readyDataByte > gap_size)
  {
    printf("\n\n");
    niceprint = 0;
  }
  
  return 0;

}

*/



void add_pulses(int pcount)
{
  local_buffer[write_local_buffer++] = pcount;
}

void add_space(int slen)
{
  local_buffer[write_local_buffer++] = slen;
}


void displayAndPipeSignal(unsigned char * hexString, int len, int gap)
{
  FILE * pFile;
  unsigned char * freqArg;
  float freq;
#ifndef WIN
  int wnum;
#endif


  freqArg = &hexString[5];
//  freq = (float)1000000/((float)convert_quad(freqArg, 4)*0.241246);
  freq = (float)((1000000/0.241246)  / (convert_quad(freqArg, 4)));
  
#ifndef WIN
  if(pipeOnly == 1 && pipePtr > 0) {
    /* Make this easier */    
    wnum = write(pipePtr, hexString, len);
  }
#endif
  
  
  if(standalone == STANDALONE_TIMING) {
    printf("%s", hexString);
    return;
  }
  
  if(displayOnly == 1) {
    // Write to screen only
    if(gap > 0 && (addGap == 1))
      printf("Code at %5.2fkHz (gap %3.1f ms): %s", freq/1000, (float)gap/1000, hexString);
    else
      printf("Code at %5.2fkHz: %s", freq/1000, hexString);
  }
  
    
  if(displayOnly + pipeOnly == 0) {
    // Regular, write to file
    pFile = fopen (rec_filename == NULL ? "testrec.txt" : rec_filename,"w");
    if(pFile==NULL)
    {
      printf("Error opening output file.\n");
      exit(0);
    }
    
    /* hexString already has a terminating \n */
    if(gap > 0)
      printf("Code at %5.2fkHz, gap %3.1f ms (Press 'Enter' to write to '%s'): %s", freq/1000, (float)gap/1000.0, rec_filename, hexString);
    else
      printf("Code at %5.2fkHz (Press 'Enter' to write to '%s'): %s", (float)freq/1000, rec_filename, hexString);
    fprintf(pFile, "%s", hexString);
    fclose (pFile);
  }
  
}


void dump_signal(int gap)
{
  int snum;
  int freq;
  int fileFreq;

  if(currentPCA_Frequency == 0)
  {
//    printf("Frequency not set\n");
    write_local_buffer = 0;
    return;
    
  }

  freq = 1000000/(currentPCA_Frequency/48);
  fileFreq = (int)(1000000/ (freq * 0.241246) );

  // makeHexString creates internalPipeBuffer
  snum = makeHexString(fileFreq, currentPCA_Frequency, gap);

  if(tcpRxListenersCount > 0) {
    // Don't know if the listeners are TCP or Pipe, to send to both:
////// this already has it    internalPipeBuffer[snum++] = '\0';    Nov10
#ifndef WIN    
    addTcpRxData(internalPipeBuffer, snum, 'R');
    pipeRxSend(snum);
#endif
//    return;  // Can write to pipe even if writing to network (file is probably optional)
  } else {
    // Pipe and TCP should both connect
    displayAndPipeSignal(internalPipeBuffer, snum, gap);
  }
//  currentPCA_Frequency = 0;
  write_local_buffer = 0;
}


int makeHexString(
  int fileFreq, 
  int currentPCA_Frequency,
  int gapMicroseconds) {
  
  int slen = 0, snum, x;
  int gapPulseCounts;
  
  
  if(addGap == 0)
    gapMicroseconds = 0;  // clear the gap, since they do not want it
  
  if(gapOnly == 1)
  {
    if(gapMicroseconds == 0)
      printf("(valid gap not detected)\n");
    gapPulseCounts = 48 * gapMicroseconds / currentPCA_Frequency;
    snum = sprintf(&internalPipeBuffer[slen], "Gap: %04X (%d microseconds)\n", 
      gapPulseCounts, gapMicroseconds);
    slen = snum;  // Keep convention in case we have more to add
    return slen;
  } 
  
  // Else, gapOnly == 0:
 
  snum = sprintf(internalPipeBuffer, "0000 %04X %04X %04X ", 
    fileFreq, write_local_buffer + (gapMicroseconds > 0 ? 1 : 0), 0);
  slen = snum;
  for(x = 0; x < write_local_buffer; x++)
  {
    snum = sprintf(&internalPipeBuffer[slen], "%04X ", (x % 2 == 0) ? 
      local_buffer[x] : local_buffer[x] * 48 / currentPCA_Frequency );
    slen+=snum;
  }
  if(gapMicroseconds > 0)
  {
    gapPulseCounts = 48 * gapMicroseconds / currentPCA_Frequency;
    snum = sprintf(&internalPipeBuffer[slen], "%04X ", gapPulseCounts );
    slen+=snum;
  }      
  slen--; // Take off trailing space
  snum = sprintf(&internalPipeBuffer[slen], "\n\0");  // EOL
  slen+=snum;
  return slen;
}
  

// Originally written in LIRC's hw_commandir.c
int commandir3_convert_RX(unsigned char *rxBuffer,
	int numNewValues)
{
	static unsigned char incomingBuffer[MAX_INCOMING_BUFFER + 30];
	static unsigned char switchByte = 0;
	static int incomingBuffer_Write = 0, incomingBuffer_Read = 0;
	static int	buffer_write;
	static float currentPWM = 0;
	
	int i, mySize;
	int currentSize, expectingBytes;
	int packet_number;
	static int mcu_rx_top_location;
	static int pulse_count = 0;
	
	struct usb_rx_pulse3 * a_usb_rx_pulse;
	struct usb_rx_space3 * a_usb_rx_space;
	struct usb_rx_pulse_def3 * a_usb_rx_pulse_def;
	struct usb_rx_demod_pulse * a_usb_rx_demod_pulse;
	
	if(numNewValues > 0)
	{
		packet_number = rxBuffer[0];
		expectingBytes = rxBuffer[1] + rxBuffer[2] * 256;
		if(numNewValues != (expectingBytes + 5))
			printf("MCU top is now: %d (a change of: %d)\n", 
				rxBuffer[3] + rxBuffer[4] * 256, (rxBuffer[3] + rxBuffer[4] * 256) - 
				mcu_rx_top_location);
		mcu_rx_top_location = rxBuffer[3] + rxBuffer[4] * 256;
		
		if(numNewValues != (expectingBytes + 5))
			printf(
				"\nUSB received: %d, expectingBytes: %d. Hex data (headers: %d %d %d), incomingBuffer_Read: %d\n"
				, numNewValues, expectingBytes, rxBuffer[0], rxBuffer[1], rxBuffer[2], incomingBuffer_Read);
		for (i = 5; i < expectingBytes + 5; i++)
		{
			if(numNewValues != (expectingBytes + 5)) 	
				printf("%02x@%d\t", rxBuffer[i], incomingBuffer_Write);
 
			incomingBuffer[incomingBuffer_Write++] = rxBuffer[i];
			if(incomingBuffer_Write >= MAX_INCOMING_BUFFER)
				incomingBuffer_Write = 0;
		}
		
		if(incomingBuffer_Write > incomingBuffer_Read)
		{
			currentSize = incomingBuffer_Write - incomingBuffer_Read;
		} else {
			currentSize = (MAX_INCOMING_BUFFER - incomingBuffer_Read) + 
				incomingBuffer_Write;
		}
		
		while(currentSize > 0)
		{
			// Make sure the entire struct is received before trying to use
			switch(incomingBuffer[incomingBuffer_Read])
			{
				case USB_RX_PULSE:
					mySize = 2;
					break;
				case USB_RX_SPACE:
					mySize = 4;
					break;
				case USB_RX_PULSE_DEMOD:
					mySize = 4;
					break;
				case USB_RX_SPACE_DEMOD:
					mySize = 4;
					break;
				case USB_RX_PULSE_DEF:
					mySize = 4;
					break;
				case USB_NO_DATA_BYTE:
//					read_num = write(child_pipe_write, lirc_zero_buffer, 
//						sizeof(lirc_t)*insert_fast_zeros);
					mySize = 0; // we should IGNORE these; first data is always a ZERO right now
					break;
				default:
					printf("Unknown struct identifier: %02x at %d\n", 
						incomingBuffer[incomingBuffer_Read], incomingBuffer_Read);
					mySize = 0;
					break;
			}
			
			if(currentSize >= (mySize+1))	
			{
				if(incomingBuffer_Read + mySize >= MAX_INCOMING_BUFFER)
				{
					memcpy(&incomingBuffer[MAX_INCOMING_BUFFER], &incomingBuffer[0], 
						mySize + 1);	// +1 to compensate for header
				}
				
				switchByte = incomingBuffer[incomingBuffer_Read++];
				if(incomingBuffer_Read >= MAX_INCOMING_BUFFER)
					incomingBuffer_Read -= MAX_INCOMING_BUFFER;
				currentSize--;
				
				buffer_write = 0;

				switch(switchByte)
				{
				case USB_NO_DATA_BYTE:
#ifdef WIN32
					buffer_write = 0;
					lirc_pipe_write(&buffer_write);
					lirc_pipe_write(&buffer_write);
#endif
					  if(pulse_count > 0) {
					  	if(standalone != STANDALONE_TIMING)
	    					dump_signal( 0 ); // timeout before gap
    			  }
					  pulse_count = 0;
//          }
					break;
					
				case USB_RX_PULSE:
					a_usb_rx_pulse = (struct usb_rx_pulse3 *)
						&incomingBuffer[incomingBuffer_Read];

					buffer_write =  (int)(a_usb_rx_pulse->t0_count * 
						(1000000/ (1/( ((float)(currentPCA_Frequency))/48000000) ) ) );
					buffer_write |= PULSE_BIT; 
#ifdef WIN32
					lirc_pipe_write(&buffer_write); 
#else
					add_pulses(a_usb_rx_pulse->t0_count);
#endif
					buffer_write -= PULSE_BIT;
					pulse_count++;
					break;
					
				case USB_RX_SPACE:
					a_usb_rx_space = (struct usb_rx_space3 *)
						&incomingBuffer[incomingBuffer_Read];
					buffer_write = (a_usb_rx_space->pca_offset + 
						(a_usb_rx_space->pca_overflow_count) * 0xffff)/48;
#ifdef WIN32
					lirc_pipe_write(&buffer_write);  //	Return this for raw RX
#else
					if(buffer_write < (gap_size) ) // Allow lots of space for error in gap size
					  add_space(buffer_write);
					else {
					  if(standalone != STANDALONE_TIMING)
					  {
					    if(pulse_count > 0)
    					  dump_signal( ( (buffer_write > gap_size) && (buffer_write < 175000) )? buffer_write : 0 );
  					  }
					  pulse_count = 0;
					}
#endif
					break;
					
				case USB_RX_PULSE_DEF:
					a_usb_rx_pulse_def = (struct usb_rx_pulse_def3 *)
						&incomingBuffer[incomingBuffer_Read];
					currentPWM = ((float)(a_usb_rx_pulse_def->pwm))/48;
/*					printf(" Pulse Def, modulation frequency: %f us, pwm: %fus; Duty Cycle: %f%%\n",
							1/( ((float)(a_usb_rx_pulse_def->frequency))/48000000),
							currentPWM,
							100* ((((float)(a_usb_rx_pulse_def->pwm))/48) / 
								(((float)(a_usb_rx_pulse_def->frequency)/48)) ));
								*/
					currentPCA_Frequency = a_usb_rx_pulse_def->frequency;
					break;
					
				case USB_RX_PULSE_DEMOD:
					a_usb_rx_demod_pulse = (struct usb_rx_demod_pulse*)
						&incomingBuffer[incomingBuffer_Read];
					buffer_write = (a_usb_rx_demod_pulse->pca_offset + 
						(a_usb_rx_demod_pulse->pca_overflow_count) * 0xffff)/48;
					buffer_write |= PULSE_BIT;
#ifdef WIN32
					lirc_pipe_write(&buffer_write);
#endif
				    buffer_write -= PULSE_BIT;
					break;
					
				case USB_RX_SPACE_DEMOD:
					a_usb_rx_demod_pulse = (struct usb_rx_demod_pulse*)
						&incomingBuffer[incomingBuffer_Read];
					buffer_write = (a_usb_rx_demod_pulse->pca_offset + 
						(a_usb_rx_demod_pulse->pca_overflow_count) * 0xffff)/48;
#ifdef WIN32
					lirc_pipe_write(&buffer_write);
#endif
					break;
				}
				
#ifndef WIN
	  		if(standalone == STANDALONE_TIMING || (tcpTimingListenersCount > 0))
			  {
			    timingEvent(switchByte, buffer_write);
			  }
#endif
				incomingBuffer_Read +=mySize;
				if(incomingBuffer_Read >= MAX_INCOMING_BUFFER)
					incomingBuffer_Read -= MAX_INCOMING_BUFFER;
				currentSize -= mySize;
			} else {
				break; // from while
			}
		}
	}
	return 0;
}

/*** New pdata functions ***/



unsigned char * getCommandIR_pdataRaw(usb_dev_handle * c) {
  // Send Request
  
  // The request packet is different than usb_pdata!
  unsigned char pdataRequestPacket[4] = {5,0,0,4};  // the MCU receives this as a struct
  int read_retval, send_status, x;
  unsigned char * incoming_packet;
  // 5 = header id, 4 = bytes in packet.
  
#ifdef ENABLE_TEST_NULL_PDATA
  printf("Simulating no pdata available: Returning NULL\n");
  return NULL;
#endif
  
  incoming_packet = malloc(64);
  
  send_status=usb_bulk_write(
	  c, 
	  1,  // which is the firmware endpoint
	  pdataRequestPacket,
	  sizeof(pdataRequestPacket),
	  500);


  // Loop for Response
  // try 10 times to receive the response; it should be ~#1-3
  for(x=0; x<10; x++) {
    read_retval = usb_bulk_read(
      c,
      0x81, // which is the RX-data endpoint?
      incoming_packet,  // ensure the proper header
      64,
      5000);
    if(read_retval > 0) {
      // How is the pdata identified by the MCU?
      if(read_retval > 8) {
        // This is the header for a usb_pdata packet, containing 56 bytes of pdata that we should extract.
//        printf("Received a non-8-byte packet (%d bytes)\n", read_retval);
        return incoming_packet; // bytes 0-3 are header, but we better return the whole thing.
      } 
    }
  }
  printf("No persistent data returned from CommandIR. (Firmware >=3.42 is required)\n");
  printf  ("Last packet returned from CommandIR was %d bytes (expeceting 8 bytes).\n", read_retval);
  if(read_retval >=4)
    printf("Received a packet that was not pdata (byte3 is %0X, length was %d)\n", incoming_packet[3], read_retval);
  return NULL;
}

unsigned char * getCommandIR_pdata(usb_dev_handle * c) {
  // Chop off the header
  unsigned char * d;
  d = getCommandIR_pdataRaw(c);
  if(d == NULL)
    return NULL;
  return &d[4];
}

void setCommandIR_Order(usb_dev_handle * c, int order_num) {
  // Change a specific struct item in the persistent area
  unsigned char * pdata;
  pdata = getCommandIR_pdata(c);
  ((struct persistent_data_03 *)pdata)->order_num = order_num;
  setCommandIR_pdata(c, (unsigned char *)pdata);
}

void setCommandIR_Mode(usb_dev_handle * c, int mode) {
  // Change a specific struct item in the persistent area
  unsigned char * pdata;
  pdata = getCommandIR_pdata(c);
  ((struct persistent_data_03 *)pdata)->operation_mode = mode;
  setCommandIR_pdata(c, (unsigned char *)pdata);
}

void setCommandIR_Name(usb_dev_handle * c, unsigned char * name) {
  // Change a specific struct item in the persistent area
  unsigned char * pdata;   // This is VERSION DEPENDENT.
  int x;
  int copyLen;
  pdata = getCommandIR_pdata(c);
  // Copy the string; note the limits.
  copyLen = strlen(name);
  if(copyLen > 9)
    copyLen = 9;  // we enforce zero-terminated string for the 10th byte
  for(x=0;x<copyLen;x++) {
    ((struct persistent_data_03 *)pdata)->name[x] = name[x];
  }
  ((struct persistent_data_03 *)pdata)->name[x] = '\0';  // zero-terminate
          printf("Set CommandIR name (in struct) to %s\n",((struct persistent_data_03 *)pdata)->name);

  setCommandIR_pdata(c, pdata);
}

void setCommandIR_Group(usb_dev_handle * c, unsigned char * name) {
  // Change a specific struct item in the persistent area
  unsigned char * pdata;
  int x;
  int copyLen;
  pdata = getCommandIR_pdata(c);
  // Copy the string; note the limits.
  copyLen = strlen(name);
  if(copyLen > 9)
    copyLen = 9;  // we enforce zero-terminated string for the 10th byte
  for(x=0;x<copyLen;x++) {
    ((struct persistent_data_03 *)pdata)->group[x] = name[x];
  }
  ((struct persistent_data_03 *)pdata)->group[x] = '\0';  // zero-terminate
  setCommandIR_pdata(c, pdata);
}

void setCommandIR_pdata(usb_dev_handle * c, unsigned char * pdata) {
  // write pdata to the USB
  struct usb_pdata send_usb_pdata;
  int x, send_status;
  
  send_usb_pdata.header_id = 127;  // 127 or 5 ?? 
  send_usb_pdata.pdata_format_id = 3;
  send_usb_pdata.placeholder = 0;
  send_usb_pdata.num_bytes = 60;
  // copy pdata into send_usb_pdata->pdata
  for(x=0;x<56;x++) {
    send_usb_pdata.pdata[x] = pdata[x];
  }
  
  send_status=usb_bulk_write(
	  c, 
	  1, 
	  (unsigned char*)&send_usb_pdata,
	  sizeof(send_usb_pdata),
	  500);
  // The commandir should reset after writing the update.
  // graceful_exit?  No, might be updating OTHER CommandIRs
}

void pdataDisplay(unsigned char * pdata_raw) {
  struct persistent_data_03 * pdata_03; 
  if(pdata_raw == NULL) 
    return;
  
  switch(pdata_raw[1]) {  // Version Byte
    case 3: // Current version
      pdata_03 = (void *)&pdata_raw[4];
      printf(" pdata Version: %d\n", pdata_raw[1]);
      printf(" CommandIR Order: %d\n", pdata_03->order_num);
      printf(" CommandIR Name: %s\n", pdata_03->name);
      printf(" CommandIR Group: %s\n", pdata_03->group); 
      switch(pdata_03->operation_mode) {
        case 0: // Mini mode
          printf(" CommandIR Mode: Mini\n");  
          break;
        case 1: // Expansion mode
          printf(" CommandIR Mode: Expanded\n");  
          break;
      }        
      break;
  }
}

void clear_order() {
	struct commandir_tx_order * ptx, * next_ptx;
	ptx = ordered_commandir_devices;
	while(ptx) {
		next_ptx = ptx->next;
		free(ptx);
		ptx = next_ptx;
	}
	ordered_commandir_devices = NULL;	// we just deleted them all!
}


void hardware_setorder() {
	// This is different now because we're using linked lists.
	// Each commandir_device object has a busnum and devnum; we basically sort these.
	struct commandir_device * pcd;
	int CommandIR_Num = 0;
	int emitters = 1;
  int this_pcd_value;

  clear_order();
  
  // foreach pcd, insert into tx_order linked list
  if(first_commandir_device == NULL) return;
  
	for(pcd = first_commandir_device; pcd != NULL; pcd = pcd->next_commandir_device) {
	  this_pcd_value = pcd->busnum * 128 + pcd->devnum;
    insert_into_tx_order(this_pcd_value, pcd);
  }		
 
	if(first_commandir_device == NULL)
	  return;
	
	if(first_commandir_device->next_commandir_device)
	{
		printf("Re-ordered Multiple CommandIRs:\n");
		for(pcd = first_commandir_device; pcd; pcd = pcd->next_commandir_device) {
			printf(" CommandIR Index: %d (Type: %d, Revision: %d), Emitters #%d-%d\n", CommandIR_Num, pcd->hw_type, pcd->hw_revision, emitters, emitters + pcd->num_transmitters-1);
			CommandIR_Num++;
			emitters += pcd->num_transmitters;
		}
	}

}



void hardware_setorder_by_pdata_order() {
  // Assuming no order has been set, create the ordered_commandir_device by each device's device_id; modified to fall-back on USB order
  struct persistent_data_03 * pd;
  struct commandir_device * pcd;
  int this_pcd_value;
  
  // printf("hardware_setorder_by_pdata_order\n");
  // Clear it first.
  clear_order();
  
  // Load all pdata for the devices, and then create an order from it.
	for(pcd = first_commandir_device; pcd; pcd = pcd->next_commandir_device) {
	  // Give it a default order, based on the USB bus and device numbers:
	  this_pcd_value = pcd->busnum * 128 + pcd->devnum;
    if(pcd->pdata == NULL) {
      pcd->pdata = getCommandIR_pdata(pcd->cmdir_udev);
//      if(pcd->pdata == NULL) {
//        printf("Could not load pdata from CommandIR at USB %d:%d\n", pcd->busnum, pcd->devnum);
//        continue; // could not get this CommandIR's pdata - firmware too old?
//      }
    }
  
    if(pcd->pdata != NULL)  {
      pd = (struct persistent_data_03 *)pcd->pdata;
      // So, pd->order_num is the persistent order identifier
      // Where does it fit into the list? EASY NOW.
      this_pcd_value = pd->order_num;
    }
    insert_into_tx_order(this_pcd_value, pcd);
  }
}

// this_pcd_value depends on the ordering function we're using
// this only works if we SAVE the pcd_value that we used in the commandir_tx_order
void insert_into_tx_order(int this_pcd_value, struct commandir_device * pcd) {
  struct commandir_tx_order * ptx, * new_ptx, * last_ptx;
	int done = 0;
  int this_ptx_value; 
  
//  printf("Adding %p as this_pcd_value %d.\n", pcd, this_pcd_value);
  
  new_ptx = malloc(sizeof(struct commandir_tx_order));
  new_ptx->the_commandir_device = pcd;
  new_ptx->next = NULL;
  new_ptx->value_for_ordering = this_pcd_value; // this is what we use to order them
  
  if(ordered_commandir_devices == NULL) {
    ordered_commandir_devices = new_ptx;
    return;
  }
  
  // go down the ptx chain until we find the right place for this device
  ptx = ordered_commandir_devices;
  this_ptx_value = ptx->value_for_ordering;
  last_ptx = NULL;
  
  if(this_pcd_value > this_ptx_value) {
	   // We're already the highest item:
	   ptx->next = new_ptx;
	   return;
  }

  if(this_pcd_value < this_ptx_value) {
	   // We're already the smallest item:
	   new_ptx->next = ordered_commandir_devices;
	   ordered_commandir_devices = new_ptx;
	   return;
  }

  while(this_pcd_value < this_ptx_value) {
    // keep moving up the ptx chain until pcd is great, then insert
    if(ptx->next == NULL) {
     last_ptx->next = new_ptx;
     new_ptx->next = ptx;
     done = 1;
     break; // we're at the end
    }
    last_ptx = ptx;
    ptx = ptx->next;
    if(ptx==NULL) {
    // Also adding to the end of the list, we have no more items in the chain!
     ptx->next = new_ptx;
     done = 1;
     break; // we're at the end
    }
	}
	// not sure why we were RE-calculating this here?
  //	 this_ptx_value = ptx->the_commandir_device->busnum * 128 + ptx->the_commandir_device->devnum;
	   

	// Did we stop because we have no more ptx, or because pcd_value is > ptx_value ??

	// At this point, pcd_value > ptx value, so we insert BEFORE PTX using last_ptx
	// last_ptx->next = ptx
	// ptx->next = ? : null
	if(!done) {
	  if(last_ptx==NULL) {
			// this one is the new beginning:
			new_ptx->next = ordered_commandir_devices;
			ordered_commandir_devices = new_ptx;
			return;
	  }

	  // Otherwise, we're either adding to the end (ptx->next = new_ptx) or splicing (last_ptx->next = new_ptx && new_ptx->next = ptx;)
		 // HOW DO WE KNOW?

	  last_ptx->next = new_ptx;
	  new_ptx->next = ptx;
	}
}

