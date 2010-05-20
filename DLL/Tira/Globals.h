#ifndef GLOBALS_H
#define GLOBALS_H

extern HANDLE threadExitEvent;
extern HANDLE dataReadyEvent;

extern CRITICAL_SECTION criticalSection;

extern struct mytimeval start,end,last;

void waitTillDataIsReady(int maxUSecs);
int gettimeofday(struct mytimeval *a, void *);

#endif