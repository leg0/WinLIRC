#include "stdafx.h"
#include "ConfigDialog.h"
#include "irdriver.h"

#include <winlirc/PluginApi.h>

static std::unique_ptr<irtiny::CIRDriver> irDriver;
winlirc_interface Winlirc;

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

extern "C" static int irtiny_init(winlirc_interface const* wl)
{
    Winlirc = *wl;
    Winlirc.init_rec_buffer(&rec_buffer);
    irDriver = std::make_unique<irtiny::CIRDriver>(CreateEvent(nullptr, TRUE, FALSE, nullptr));
    return irDriver->initPort();
}

extern "C" static void irtiny_deinit()
{
    if (irDriver) irDriver->signalDone();
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

static int winlirc_receive_decode(rbuf* r, hardware const* hw, ir_remote* remote,
    ir_code* prep, ir_code* codep, ir_code* postp,
    int* repeat_flag,
    lirc_t* min_remaining_gapp,
    lirc_t* max_remaining_gapp)
{
    return Winlirc.receive_decode(r, hw, remote, prep, codep, postp, repeat_flag, min_remaining_gapp, max_remaining_gapp);
}

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

    Winlirc.clear_rec_buffer(&rec_buffer, &irtiny_hardware);
    return Winlirc.decodeCommand(&rec_buffer, &irtiny_hardware, remotes, out, out_size);
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
