#ifndef LIBCMDIR_H
#define LIBCMDIR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef WIN
#include "libusb/include/usb.h"
#else
#include <usb.h>
#endif

#ifndef WIN32
int kbhit (void);
#pragma BEGINDUMP
#pragma comment( lib, "libusb\lib\bcc\libusb.lib" )
#pragma ENDDUMP
#endif

#include <fcntl.h>

#define MAX_INCOMING_BUFFER 65535

#define USB_RX_PULSE_DEF 31
#define USB_RX_PULSE 32
#define USB_RX_SPACE 33
#define USB_RX_PULSE_DEMOD 34
#define USB_RX_SPACE_DEMOD 35
#define USB_NO_DATA_BYTE 36

#define STANDALONE_SEND 1
#define STANDALONE_RECORD 2
#define STANDALONE_TIMING 3
#define STANDALONE_SET 4

/* Prototypes  */
void displayAndPipeSignal(unsigned char * hexString, int len, int gap);
int commandir_rec(usb_dev_handle * udev);
int hardware_scan(void);
int commandir3_convert_RX(unsigned char *rxBuffer,int numNewValues);
void dump_signal(int gap);
struct sendir * tx_char_2_struct(unsigned char * buffer);
void commandir_sendir(struct sendir * tx);
char * load_file(char * filename);
void transmit_tx_char(unsigned char * tx_char);
void set_all_bitmask(unsigned char d);
int convert_quad(char * a, int len);
void disconnectUnneededCommandIRs(void);
int hardware_check_changes(void);
void clear_order(void);
void set_long_tx_bit(int bitnum);
void convert_set_long(char * a, int len);
void detect_tx_per_commandir(void);
void reserveCommandIR_By_Name(unsigned char * name) ;
void hardware_setorder_by_pdata_order(void);
void connectReservedCommandIRs(void) ;
int commandir_timings(usb_dev_handle * udev);
int get_frequency_from_hex(unsigned char * hexarg);

void hardware_setorder(void);
void set_transmitters_int(unsigned int t_enable);
struct commandir_device * addCommandIR(struct usb_device *dev, struct usb_bus *bus);
void hardware_disconnect(struct commandir_device *a);
void commandir_disconnect(struct commandir_device * pcd);
void software_disconnects(void);
void check_commandir_add_remove();

/* CommandIR Internal Structs */
struct commandir_3_tx_signal
{
	unsigned short tx_bit_mask1;
	unsigned short tx_bit_mask2;
	unsigned short tx_min_gap;
	unsigned short tx_signal_count;
	unsigned short pulse_width;
	unsigned short pwm_offset;
};

struct usb_rx_space3 {
 unsigned short pca_overflow_count;
 unsigned short pca_offset;
};

struct usb_rx_pulse3 {
 unsigned short t0_count;
};

struct usb_rx_pulse_def3 {
 unsigned short frequency;
 unsigned short pwm;
};

struct usb_rx_demod_pulse {
 unsigned short pca_overflow_count;
 unsigned short pca_offset;
};

/* Struct for commandir_send()  */
struct sendir {
  int byteCount;
  char * buffer;
  int emitterMask;
};

struct commandirIII_status {
	unsigned char jack_status[4];
	unsigned char rx_status;
	unsigned char tx_status;
	unsigned char versionByte;
	unsigned char expansionByte;
};

#define USB_CMDIR_VENDOR_ID		0x10c4

#ifdef WIN32
#define PULSE_BIT  0x01000000
#else
#define PULSE_BIT 0x8000
#endif
/* New for v0.3 */
usb_dev_handle * hardware_scan_for(int busNum, int devId);

struct usb_pdata {
  unsigned char header_id;
  unsigned char pdata_format_id;
  unsigned char placeholder;
  unsigned char num_bytes;
  unsigned char pdata[56];  // this may change (on our end) depending on the pdata_format_id
};

struct persistent_data_03 {
  unsigned char pdata_format_version_id;  // 3 for this one, "03"
  unsigned char order_num;  // What order position we take in the group
  unsigned char name[10];   // 9 bytes + NULL
  unsigned char group[10];  // 9 bytes + NULL
  unsigned char operation_mode; 
};

struct commandir_tx_order
{
  struct commandir_device * the_commandir_device;
  struct commandir_tx_order * next;
  unsigned char * reservedName; // New for Utils 0.5
  int value_for_ordering;
};

struct commandir_device
{
	usb_dev_handle *cmdir_udev;
	unsigned char * pdata;
	int interface;
	int hw_type;
	int hw_revision;
	int hw_subversion;
	int busnum;
	int devnum;
// 	int order_id;	// busnum * 128 + devnum -> do we really have to store it?
	int endpoint_max[4];
	int num_transmitters;
	int num_receivers;
	int num_timers;
	int tx_jack_sense;
	unsigned char stillFound;   // for checking whether devices are still attached or not
	unsigned char rx_jack_sense;
	unsigned char rx_data_available;

// 	int next_tx_mask;	// This is in low-bits-first format; generic to all hardware
	int * next_enabled_emitters_list;	// when we create a new commandir_device, set this to [num_transmitters]
	int num_next_enabled_emitters;
	char signalid;

	struct tx_signal * next_tx_signal;
	struct tx_signal * last_tx_signal;

	unsigned char lastSendSignalID;
	unsigned char commandir_last_signal_id;
	unsigned char flush_buffer;	// Flush any incoming RX data that's probably stale in the CommandIR's on-board memory

	// CommandIR Mini Specific:
	int mini_freq;
	unsigned char commandir_tx_available;
	
	// New for CommandIR Utils 0.5
	unsigned char flag_disconnect;
	

	// ALL CommandIRs have 4 TX channels, so ultimately we're just going to store what's in 4 buffers
	// But with CommandIR III, we only need ONE of these, since we send data to the external RAM for queuing
	// The next_data[4] in the firmware will point to RAM, and we'll have to deal with the cases where
	// the first signal to finish TXing is NOT THE TOP SIGNAL in the queue!  So when the top finishes, the
	// "free" pointer has to move TWO spaces back, not just one.

//	unsigned char commandir_tx_start[MAX_TX_TIMERS*4];
//	unsigned char commandir_tx_end[MAX_TX_TIMERS*4];
//	unsigned char commandir_tx_available[MAX_TX_TIMERS];

	// What's changed is CommandIR III Pro supports 10 transmitters, with the 4 tx timers.
	// How do we map them?  Which TX emitter is getting the timer output
//	unsigned char tx_timer_to_channel_map[MAX_TX_TIMERS];

	struct commandir_device * next_commandir_device;
};


void commandir_send_specific(struct sendir * tx, struct commandir_device * pcd);
struct commandir_device * getCommandIR_By_Name(unsigned char * n);
void insert_into_tx_order(int this_pcd_value, struct commandir_device * pcd);
void hardware_setorder_by_pdata_order();
void clear_order();
void pdataDisplay(unsigned char * pdata_raw);
void setCommandIR_pdata(usb_dev_handle * c, unsigned char * pdata);
void setCommandIR_Group(usb_dev_handle * c, unsigned char * name);
void setCommandIR_Name(usb_dev_handle * c, unsigned char * name);
void setCommandIR_Mode(usb_dev_handle * c, int mode);
void setCommandIR_Order(usb_dev_handle * c, int order_num);
unsigned char * getCommandIR_pdata(usb_dev_handle * c);
unsigned char * getCommandIR_pdataRaw(usb_dev_handle * c);


/* Utils 0.5 */
extern unsigned char internalPipeBuffer[4096];  // libcmdir
extern int tcpRxListenersCount; // libtcp
extern unsigned char addGap;
extern unsigned char gapOnly;

struct tx_signal {
	char *raw_signal;
	int raw_signal_len;
	int raw_signal_tx_bitmask;
	int *bitmask_emitters_list;
	int num_bitmask_emitters_list;
	int raw_signal_frequency;
	struct tx_signal *next;
};

/* Prototypes that depend on an above struct */
int commandir_send(char * packet, int bytes, struct commandir_device * pcd);

//
// externs
//

extern struct commandir_tx_order * ordered_commandir_devices;
extern unsigned char standalone;
extern unsigned char pipeOnly;
extern unsigned char displayOnly;
extern struct commandir_device * first_commandir_device;

#endif



	

