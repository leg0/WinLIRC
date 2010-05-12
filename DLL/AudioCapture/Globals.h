#ifndef GLOBALS_H
#define GLOBALS_H

#include "RecordAudio.h"
#include "AnalyseAudio.h"
#include "Settings.h"

extern RecordAudio	*recordAudio;
extern AnalyseAudio	*analyseAudio;
extern Settings		*settings;
extern HANDLE		dataReadyEvent;
extern HANDLE		threadExitEvent;

void waitTillDataIsReady(int maxUSecs);

#endif