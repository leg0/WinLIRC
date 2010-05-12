#ifndef GLOBALS_H
#define GLOBALS_H

#include "Settings.h"
#include "irdriver.h"

extern Settings		settings;
extern HANDLE		threadExitEvent;
extern CIRDriver	*irDriver;

void KillThread(CWinThread **ThreadHandle, CEvent *ThreadEvent);

#define HARDCARRIER		0x0001		/* the transmitter generates its own carrier modulation */
#define TXTRANSMITTER	0x0002		/* the transmitter uses the TX pin */
#define INVERTED		0x0004		/* the transmitter polarity is opposite */

#endif