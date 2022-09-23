// SDK for BeholdRC.dll
// Version 2.2 ( updated )
//
// Support:
// http://www.beholder.ru
// mailto:support@beholder.ru

#pragma once

#include "windows.h"
#include <tchar.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

  int   BTV_GetIStatus  (void);      // Get interface status: 0 - Library not loaded, 1 - WDM device not selected, 2 - OK.
  BOOL  BTV_SelectCard  (int idx=0); // Select Beholder WDM device.
  int   BTV_GetRCCode   (void);      // Get remote control key code, short format. Works only with native remote control.
                                     // Returns -1 if no key pressed. Otherwise returns one byte key code.
  ULONG BTV_GetRCCodeEx (void);      // Get remote control key code, long format. Returns code for any remote control supported by hardware.
                                     // Returns 0 if no key pressed. Otherwise returns four bytes key code.

#ifdef __cplusplus
}
#endif

/////////////////////////////////////////////////////////////////////////////
//
// ATTENTION:
// Before you can use BTV_GetRCCode function, you must select the TV tuner
// card. If there is only one Beholder tuner in the system, just call
// BTV_SelectCard() without parameters. If there are no tuners in the system
// or error occured during initialization, this function will return FALSE.
// If initialization is successfull it returns TRUE.
//
// Poll the BTV_GetRCCode or BTV_GetRCCodeEx by timer or use the separate
// thread. The optimal poll interval is 30-50ms.
//
/////////////////////////////////////////////////////////////////////////////
//
// *** The SHORT codes for Beholder remote control units ***
//
/////////////////////////////////////////////////////////////////////////////
//
// Old style 30 keys remote control unit
//
//   -1  -  No button pressed
//    0  -  Button 0
//    1  -  Button 1
//    2  -  Button 2
//    3  -  Button 3
//    4  -  Button 4
//    5  -  Button 5
//    6  -  Button 6
//    7  -  Button 7
//    8  -  Button 8
//    9  -  Button 9
//   10  -  Recall
//   11  -  Up arrow
//   12  -  Right arrow
//   13  -  Mode
//   14  -  Sleep
//   15  -  Audio
//   16  -  Info
//   17  -  TV/AV
//   18  -  Power
//   19  -  Mute
//   20  -  Menu
//   21  -  Down arrow
//   22  -  OK
//   23  -  +100
//   24  -  Left arrow
//   26  -  Chan +
//   27  -  Vol +
//   28  -  Function
//   30  -  Chan -
//   31  -  Vol -
//
/////////////////////////////////////////////////////////////////////////////
//
// Flat 28 keys remote control unit
//
//   -1  -  No button pressed
//    0  -  Button 0
//    1  -  Button 1
//    2  -  Button 2
//    3  -  Button 3
//    4  -  Button 4
//    5  -  Button 5
//    6  -  Button 6
//    7  -  Button 7
//    8  -  Button 8
//    9  -  Button 9
//   10  -  Recall
//   11  -  CH UP
//   12  -  VOL+
//   13  -  Stereo
//   14  -  STOP
//   15  -  PREV
//   16  -  ZOOM
//   17  -  Source
//   18  -  Power
//   19  -  Mute
//   21  -  CH DOWN
//   24  -  VOL-
//   25  -  SNAPSHOT
//   26  -  NEXT
//   27  -  TIME SHIFT
//   28  -  FM Radio
//   29  -  REC
//   30  -  PAUSE
//
/////////////////////////////////////////////////////////////////////////////
//
// New style 34 keys remote control unit
//
//   -1  -  No button pressed
//    0  -  Button 0
//    1  -  Button 1
//    2  -  Button 2
//    3  -  Button 3
//    4  -  Button 4
//    5  -  Button 5
//    6  -  Button 6
//    7  -  Button 7
//    8  -  Button 8
//    9  -  Button 9
//   10  -  Recall
//   11  -  Ch Up
//   12  -  Vol +
//   13  -  Info
//   14  -  Teletext
//   15  -  Record
//   16  -  Full screen
//   17  -  Mute
//   18  -  Power
//   19  -  Preview
//   20  -  Aspect
//   21  -  Ch Down
//   22  -  OK
//   23  -  Mode
//   24  -  Vol -
//   25  -  DVB
//   26  -  Stop
//   27  -  Play/Pause
//   28  -  TV/FM
//   29  -  Sleep
//   30  -  Source
//   31  -  Audio
//   88  -  Snapshot
//   92  -  Freeze
//
/////////////////////////////////////////////////////////////////////////////
