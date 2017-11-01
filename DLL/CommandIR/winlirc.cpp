#include <Windows.h>
#include "winlirc.h"
#include "SendReceiveData.h"
#include "../Common/Win32Helpers.h"

extern "C" {
	#include "CommandIRUtils\libcmdir.h"
}

//
// local variables
//
unsigned int	currentTransmitterMask	= 0x0f;
int				doneReceiving			= 0;
HANDLE			threadHandle			= nullptr;

DWORD WINAPI receiveLoop(void *x) {

	while(!doneReceiving) {
		if(check_commandir_rec()) {
			continue;
		}
		else {
			Sleep(20);
		}
	}
  
	return 0;
}

void setup_libcmdir_winlirc()
{
	standalone	= 0;
	displayOnly	= 0;
	pipeOnly	= 1;
}

int init_commandir()
{

	bufferStart		= 0;
	bufferEnd		= 0;
	doneReceiving	= 0;

	//
	// setup functions
	//
	setup_libcmdir_winlirc			();
	hardware_check_changes			();
	detect_tx_per_commandir			();
	hardware_setorder_by_pdata_order();
	check_commandir_add_remove		();

	threadHandle = CreateThread( nullptr, 0, receiveLoop, nullptr, 0, nullptr );

	if(first_commandir_device) {
		return 1;
	}

	//
	// failed to find device or get handle on it
	//
	return 0;
}

void deinit_commandir() {

	doneReceiving = 1;

	while(first_commandir_device)
	{
		commandir_disconnect(first_commandir_device);
	}

	KillThread(nullptr,threadHandle);
}


void send_lirc_buffer(unsigned char *buffer, int bytes, unsigned int frequency)
{
	/* *buffer points to a buffer that will be OVERWRITTEN; malloc our copy.
	 * buffer is a LIRC_T packet for CommandIR
	 * For winlirc: drop tx_signal, use sendir (the CommandIR Utils version)
	 */
	// struct tx_signal *new_tx_signal = nullptr;
#define TX_BUFFER_SIZE 64

	int send_status;
	unsigned char packet[TX_BUFFER_SIZE + 2];
	int sent = 0, tosend = 0;
	int total_signals = 0;
	lirc_t *signals;	// have bytes/sizeof(lirc_t) signals
	int which_signal = 0;

	struct commandir_3_tx_signal *ptxiii;

	int sendNextSignalsCounter = 0;
	int packetCounter;
	short tx_value;
	char pulse_toggle = 1;

	total_signals = bytes / sizeof(lirc_t);
	signals = (lirc_t *) buffer;

	ptxiii = (struct commandir_3_tx_signal *)&packet;
	ptxiii->tx_bit_mask1 = (unsigned short)currentTransmitterMask;
	ptxiii->tx_bit_mask2 = (unsigned short)(currentTransmitterMask >> 16);
	ptxiii->tx_min_gap = 0;
	ptxiii->tx_signal_count = bytes / sizeof(lirc_t);
	ptxiii->pulse_width = 48000000 / frequency;
	ptxiii->pwm_offset = ptxiii->pulse_width / 2;

	packetCounter = sizeof(struct commandir_3_tx_signal);

	while (sendNextSignalsCounter < (bytes / (signed int)sizeof(lirc_t))) {

		while ((packetCounter < (64 - 1)) && (sendNextSignalsCounter < (bytes / (signed int)sizeof(lirc_t)))) {

			tx_value = signals[sendNextSignalsCounter++] * frequency /	1000000;
			packet[packetCounter++] = tx_value & 0xff;	// low byte
			packet[packetCounter++] = (tx_value >> 8) | (pulse_toggle << 7);
			pulse_toggle = !pulse_toggle;
		}

		send_status = usb_bulk_write(first_commandir_device->cmdir_udev, 2, (char *)packet, packetCounter, 50);
		packetCounter = 0;

		if (send_status < 0) {
			printf("Error on usb_bulk_write\n");
			hardware_scan();
			return;
		} 
	}
}

int setCommandIRTransmitters(unsigned int transmitterMask)
{
	currentTransmitterMask = transmitterMask;
	return 1;
}

//static void add_to_tx_pipeline(unsigned char *buffer, int bytes, unsigned int frequency)
struct sendir * convert_lircbuffer_to_sendir(unsigned char *buffer, int bytes, unsigned int frequency)
{
	/* *buffer points to a buffer that will be OVERWRITTEN; malloc our copy.
	 * buffer is a LIRC_T packet for CommandIR
	 */
	struct tx_signal *new_tx_signal = nullptr;		//	sendir->buffer = new_tx_signal
	struct sendir * return_sendir = nullptr;
	lirc_t *oldsignal, *newsignal;
	int x, pulse_now = 1;
	int projected_signal_length;
	short aPCAFOM = 0;
	float afPCAFOM = 0.0;
	int difference = 0;

	// We have to put in the expected CommandIR Header

	return_sendir = (struct sendir *)malloc(sizeof(struct sendir));
	new_tx_signal = (struct tx_signal *)malloc(sizeof(struct tx_signal));
	
	return_sendir->buffer = (char *)new_tx_signal;
	return_sendir->emitterMask = currentTransmitterMask;	// Send to all for now
	return_sendir->byteCount = sizeof(struct tx_signal) + bytes;	// Assuming byteCount includes all the struct data - NO, it should not.
	
	new_tx_signal->raw_signal = (char *)malloc(bytes);
	new_tx_signal->raw_signal_len = bytes;
	new_tx_signal->raw_signal_frequency = frequency;
	new_tx_signal->next = nullptr;

	afPCAFOM = (float)(6000000.0 / ((frequency > 0) ? frequency : DEFAULT_FREQ));
	aPCAFOM = (short)afPCAFOM;

	// Trim off mid-modulation pulse fragments, add to space for exact signals
	for (x = 0; x < (bytes / (signed int)sizeof(lirc_t)); x++) {
		oldsignal = (lirc_t *) & buffer[x * sizeof(lirc_t)];
		newsignal = (lirc_t *) new_tx_signal->raw_signal;
		newsignal += x;
		if (pulse_now == 1) {
			projected_signal_length = (((int)((*oldsignal * 12) / (afPCAFOM))) * aPCAFOM) / 12;
			difference = *oldsignal - projected_signal_length;
			// take off difference plus 1 full FOM cycle
			*newsignal = *oldsignal - difference - (aPCAFOM / 12);
		} else {
			if (difference != 0) {
				// Add anything subtracted from the pulse to the space
				*newsignal = *oldsignal + difference + (aPCAFOM / 12);
				difference = 0;
			}
		}
		pulse_now++;
		if (pulse_now > 1)
			pulse_now = 0;
	}
	return return_sendir;
}


//	send to SendReceiveData::setData as LIRC format timeout data

/* Imported from commandird - sync at some point */
void commandir_iii_update_status(struct commandir_device *cd)
{
	int receive_status;
	struct commandirIII_status *sptr;
	static char commandir_data_buffer[512];

	// Ready the first status that will tell us all the above info
	receive_status = usb_bulk_read(cd->cmdir_udev, 1,	// endpoint 1
				       (char *)commandir_data_buffer, cd->endpoint_max[1], 1500);

	if (receive_status == 8) {
		// Update the commandir_device with what we just received.
		sptr = (struct commandirIII_status *)commandir_data_buffer;
		cd->num_transmitters = (sptr->tx_status & 0x1F) + 1;
		cd->num_receivers = (sptr->rx_status & 0x60) >> 5;
		cd->tx_jack_sense =
		    sptr->jack_status[3] * 0x01000000 + sptr->jack_status[2] * 0x010000 +
		    sptr->jack_status[1] * 0x0100 + sptr->jack_status[0];
		cd->rx_jack_sense = sptr->rx_status & 0x03;
		cd->commandir_tx_available = (sptr->tx_status & 0x80 ? 1024 : 0);
		cd->rx_data_available = sptr->rx_status & 0x80;
		cd->hw_revision = sptr->versionByte >> 5;	// 3 high bits
		cd->hw_subversion = sptr->versionByte & 0x1F;	// 5 low bits
	}
}

int check_commandir_rec()
{
  struct commandir_device * pcd;  
  int totalRec = 0;
  int thisDevNum, flagged_disconnect = 0;
  
  if(first_commandir_device == nullptr)  return 0;
  
  for(pcd = first_commandir_device; pcd != nullptr; pcd = pcd->next_commandir_device) 
  {
    thisDevNum = commandir_rec(pcd->cmdir_udev);
    if(thisDevNum < 0) {
      pcd->flag_disconnect = 1;
      flagged_disconnect++;
    } else {
      totalRec += thisDevNum;
    }
  }
  if(flagged_disconnect > 0)
    disconnectUnneededCommandIRs();
  return totalRec;
}




/*** Add_to_tx_pipeline from hw_commandir in LIRC ***/
void add_to_tx_pipeline(unsigned char *buffer, int bytes, unsigned int frequency)
{
	/* *buffer points to a buffer that will be OVERWRITTEN; malloc our copy.
	 * buffer is a LIRC_T packet for CommandIR
	 */
	struct tx_signal *new_tx_signal = nullptr;
//	lirc_t *oldsignal, *newsignal;
	int *oldsignal, *newsignal;
	int x, pulse_now = 1;
	int projected_signal_length;
	short aPCAFOM = 0;
	float afPCAFOM = 0.0;
	int difference = 0;
	// Deliver this signal to any CommandIRs with next_tx_mask set
	struct commandir_device *pcd;

	new_tx_signal = (struct tx_signal *) malloc(sizeof(struct tx_signal));

	new_tx_signal->raw_signal = (char *)malloc(bytes);
	new_tx_signal->raw_signal_len = bytes;
	new_tx_signal->raw_signal_frequency = frequency;
	new_tx_signal->next = nullptr;

	afPCAFOM = (float)(6000000.0 / ((frequency > 0) ? frequency : DEFAULT_FREQ));
	aPCAFOM = (short)afPCAFOM;

	// Trim off mid-modulation pulse fragments, add to space for exact signals
	for (x = 0; x < (bytes / (signed int)sizeof(lirc_t)); x++) {
		oldsignal = (lirc_t *) & buffer[x * sizeof(lirc_t)];
		newsignal = (lirc_t *) new_tx_signal->raw_signal;
		newsignal += x;

		if (pulse_now == 1) {
			projected_signal_length = (((int)((*oldsignal * 12) / (afPCAFOM))) * aPCAFOM) / 12;
			difference = *oldsignal - projected_signal_length;
			// take off difference plus 1 full FOM cycle
			*newsignal = *oldsignal - difference - (aPCAFOM / 12);
		} else {
			if (difference != 0) {
				// Add anything subtracted from the pulse to the space
				*newsignal = *oldsignal + difference + (aPCAFOM / 12);
				difference = 0;
			}
		}
		pulse_now++;
		if (pulse_now > 1)
			pulse_now = 0;
	}

	// Create the tx_signal to send the CommandIR:

	for (pcd = first_commandir_device; pcd; pcd = pcd->next_commandir_device) {
		if (pcd->num_next_enabled_emitters) {
			// Make a local copy
			struct tx_signal *copy_new_signal;
			copy_new_signal = (struct tx_signal *) malloc(sizeof(struct tx_signal));
			memcpy(copy_new_signal, new_tx_signal, sizeof(struct tx_signal));

			copy_new_signal->raw_signal = (char *)malloc(bytes);
			memcpy(copy_new_signal->raw_signal, new_tx_signal->raw_signal, bytes);

			copy_new_signal->bitmask_emitters_list = (int *) malloc(pcd->num_transmitters * sizeof(int));

			memcpy(copy_new_signal->bitmask_emitters_list, pcd->next_enabled_emitters_list,
			       sizeof(int) * pcd->num_next_enabled_emitters);

			copy_new_signal->num_bitmask_emitters_list = pcd->num_next_enabled_emitters;
			copy_new_signal->raw_signal_tx_bitmask = 0xff; // get_hardware_tx_bitmask(pcd);		TEMPORARILY REMOVED

			copy_new_signal->next = nullptr;
//			set_new_signal_bitmasks(pcd, copy_new_signal);			TEMPORARILY REMOVED

			// Add to units' TX tree:
			if (pcd->last_tx_signal) {
				pcd->last_tx_signal->next = copy_new_signal;
				pcd->last_tx_signal = copy_new_signal;
			} else {
				// The first and the last (ie the only) signal
				pcd->last_tx_signal = copy_new_signal;
				pcd->next_tx_signal = copy_new_signal;
			}
		}
	}
	return;
}

