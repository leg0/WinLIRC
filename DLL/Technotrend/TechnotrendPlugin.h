#pragma once

#include <windows.h>
#include "ttusbirapiexp.h"
#include <winlirc/WLPluginAPI.h>

class TechnotrendPlugin : public plugin_interface {

public:
    ~TechnotrendPlugin();

    int		init(int deviceID, int busyLED, int powerLED);
    void	deinit();
    bool	getData(lirc_t* out);
    bool	dataReady();
    void	callBackFunction(PVOID Buf, ULONG len, USBIR_MODES IRMode, HANDLE hOpen, BYTE DevIdx);
    bool	waitTillDataIsReady(int maxUSecs);

private:

    void	setData(lirc_t data);

    //===================
    HANDLE	deviceHandle{ INVALID_HANDLE_VALUE };
    lirc_t	dataBuffer[256]{};
    UCHAR	bufferStart{};
    UCHAR	bufferEnd{};
    //===================
};
