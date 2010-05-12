#ifndef HARDWARE_H
#define HARDWARE_H

#include "LIRCDefines.h"
#include <Windows.h>

void initHardwareStruct();

//
// this hardware struct differs somewhat from the LIRC project
// but the functions we need for IR record should be there
// the rest are exported normally from the DLL
//

struct hardware
{
	char device[128];
	char name[128];

	unsigned long features;
	unsigned long send_mode;
	unsigned long rec_mode;
	unsigned long code_length;
	unsigned int resolution;

	int (*decode_func)(struct ir_remote *remote,
		ir_code *prep,ir_code *codep,ir_code *postp,
		int *repeat_flag,
		lirc_t *min_remaining_gapp,
		lirc_t *max_remaining_gapp);

	lirc_t	(*readdata)		(lirc_t timeout);
	void	(*wait_for_data)(lirc_t timeout);
	int		(*data_ready)	(void);
};

extern struct hardware hw;

#endif


