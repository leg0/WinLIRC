#include "stdafx.h"
#include "ConfigDialog.h"
#include "irdriver.h"

#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>

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

rbuf rec_buffer;

extern "C" static int irtiny_init(WLEventHandle exitEvent)
{
    winlirc_init_rec_buffer(&rec_buffer);
    irDriver = std::make_unique<irtiny::CIRDriver>(reinterpret_cast<HANDLE>(exitEvent));
    return irDriver->initPort();
}

extern "C" static void irtiny_deinit()
{
    irDriver.reset();
}

extern "C" static int irtiny_hasGui()
{
    return true;
}

extern "C" static void irtiny_loadSetupGui()
{
    irtiny::ConfigDialog dlg;
    dlg.DoModal();
}

// XXX WTF TODO: refactor common so that the functions that are hardcoded to use global variable hw take it as a parameter.
static hardware const irtiny_hardware =
{
    winlirc_plugin_api_version,
    "serial ir-tiny",
    "ir-tiny",
    LIRC_CAN_REC_MODE2, // features
    0, // send_mode
    LIRC_MODE_MODE2, //rec_mode
    0, // code_length
    0, // resolution
    &winlirc_receive_decode, // decode_func 
    &irtiny_readData, // readdata 
    &irtiny_waitForData, // wait_for_data 
    &irtiny_dataReady, // data_ready
    nullptr // get_ir_code 
};

extern "C" int static irtiny_decodeIR(ir_remote* remotes, char* out, size_t out_size)
{
    using namespace std::chrono_literals;
    if (!irDriver || !irDriver->waitTillDataIsReady(0us))
        return 0;

    winlirc_clear_rec_buffer(&rec_buffer, &irtiny_hardware);
    return winlirc_decodeCommand(&rec_buffer, &irtiny_hardware, remotes, out, out_size);
}

extern "C" hardware const* irtiny_getHardware()
{
    return &irtiny_hardware;
}

static plugin_interface const irtiny_plugin_interface
{
    winlirc_plugin_api_version,
    irtiny_init,
    irtiny_deinit,
    irtiny_hasGui,
    irtiny_loadSetupGui,
    nullptr, // sendIR
    irtiny_decodeIR,
    nullptr, // setTransmitters
    irtiny_getHardware,
    &irtiny_hardware
};

WL_API plugin_interface const* getPluginInterface()
{
    return &irtiny_plugin_interface;
}
