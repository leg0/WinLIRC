#ifndef HARDWARE_H
#define HARDWARE_H

#include "LIRCDefines.h"
#include <Windows.h>

void initHardwareStruct();

/*
struct hardware
{
	char device[128];
	int fd;
	unsigned long features;
	unsigned long send_mode;
	unsigned long rec_mode;
	unsigned long code_length;

	int (*decode_func)(struct ir_remote *remote,
			   ir_code *prep,ir_code *codep,ir_code *postp,
			   int *repeat_flag,
			   lirc_t *min_remaining_gapp,
			   lirc_t *max_remaining_gapp);

	lirc_t (*readdata)(lirc_t timeout);
	char name[128];
	
	unsigned int resolution;
};

*/

//extern struct hardware hw;


#endif
