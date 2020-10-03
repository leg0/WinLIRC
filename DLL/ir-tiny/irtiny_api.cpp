#include "stdafx.h"
#include "ConfigDialog.h"
#include "irdriver.h"

#include "../Common/LircDefines.h"
#include "../Common/Hardware.h"
#include "../Common/IRRemote.h"
#include "../Common/Receive.h"
#include "../Common/WLPluginAPI.h"

static std::unique_ptr<irtiny::CIRDriver> irDriver;

static lirc_t irtiny_readData(lirc_t timeout)
{
    if (irDriver)
        return irDriver->readData(std::chrono::microseconds{ timeout });
    else
        return 0;
}

static void irtiny_waitForData(lirc_t timeout)
{
    if (irDriver)
        irDriver->waitTillDataIsReady(std::chrono::microseconds{ timeout });
}

static int irtiny_dataReady()
{
    return irDriver && irDriver->dataReady();
}

// XXX WTF TODO: refactor common so that the functions that are hardcoded to use global variable hw take it as a parameter.
hardware hw /*irtiny_hardware*/ =
{
    "serial ir-tiny",
    "ir-tiny",
    LIRC_CAN_REC_MODE2, // features
    0, // send_mode
    LIRC_MODE_MODE2, //rec_mode
    0, // code_length
    0, // resolution
    &receive_decode, // decode_func 
    &irtiny_readData, // readdata 
    &irtiny_waitForData, // wait_for_data 
    &irtiny_dataReady, // data_ready
    nullptr // get_ir_code 
};

WL_API int init(HANDLE exitEvent)
{
    init_rec_buffer();
    irDriver.reset(new irtiny::CIRDriver(exitEvent));
    return irDriver->initPort();
}

WL_API void deinit()
{
    irDriver.reset();
}

WL_API int hasGui()
{
    return TRUE;
}

WL_API void loadSetupGui()
{
    irtiny::ConfigDialog dlg;
    dlg.DoModal();
}

WL_API int sendIR(struct ir_remote *remotes, struct ir_ncode *code, int repeats)
{
    return 0;
}

WL_API int decodeIR(struct ir_remote *remotes, char *out, size_t out_size)
{
    using namespace std::chrono_literals;
    if (!irDriver || !irDriver->waitTillDataIsReady(0us))
        return 0;

    clear_rec_buffer(&hw);
    return decodeCommand(&hw,remotes, out, out_size);
}

WL_API hardware* getHardware()
{
    return &hw; // &irtiny_hardware;
}
