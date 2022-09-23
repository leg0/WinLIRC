#pragma once

#include <winlirc/WLPluginAPI.h>

#define HARDCARRIER		0x0001		/* the transmitter generates its own carrier modulation */
#define TXTRANSMITTER	0x0002		/* the transmitter uses the TX pin */
#define INVERTED		0x0004		/* the transmitter polarity is opposite */

void	SetTransmitPort(HANDLE hCom,unsigned type);
int		Transmit(ir_ncode *data,struct ir_remote *rem, int repeats);
