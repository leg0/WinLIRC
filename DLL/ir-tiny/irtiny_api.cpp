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
    wchar_t port[32];
    winlirc_settings_get_wstring(L"ir-tiny", L"port", port, std::size(port), L"");
    irDriver = std::make_unique<irtiny::CIRDriver>(reinterpret_cast<HANDLE>(exitEvent), port);
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
static constexpr hardware irtiny_hardware =
{
    .plugin_api_version = winlirc_plugin_api_version,
    .device             = "serial ir-tiny",
    .name               = "ir-tiny",
    .features           = LIRC_CAN_REC_MODE2,
    .send_mode          = 0,
    .rec_mode           = LIRC_MODE_MODE2,
    .code_length        = 0,
    .resolution         = 0,
    .decode_func        = &winlirc_receive_decode,
    .readdata           = &irtiny_readData,
    .wait_for_data      = &irtiny_waitForData,
    .data_ready         = &irtiny_dataReady,
    .get_ir_code        = nullptr
};

extern "C" int static irtiny_decodeIR(ir_remote* remotes, size_t remotes_count, char* out, size_t out_size)
{
    using namespace std::chrono_literals;
    if (!irDriver || !irDriver->waitTillDataIsReady(0us))
        return 0;

    winlirc_clear_rec_buffer(&rec_buffer, &irtiny_hardware);
    return winlirc_decodeCommand(&rec_buffer, &irtiny_hardware, remotes, remotes_count, out, out_size);
}

extern "C" hardware const* irtiny_getHardware()
{
    return &irtiny_hardware;
}

WL_API plugin_interface const* getPluginInterface()
{
    static constexpr plugin_interface irtiny_plugin_interface
    {
        .plugin_api_version = winlirc_plugin_api_version,
        .init               = irtiny_init,
        .deinit             = irtiny_deinit,
        .hasGui             = irtiny_hasGui,
        .loadSetupGui       = irtiny_loadSetupGui,
        .sendIR             = nullptr,
        .decodeIR           = irtiny_decodeIR,
        .setTransmitters    = nullptr,
        .getHardware        = irtiny_getHardware,
        .hardware           = &irtiny_hardware
    };
    return &irtiny_plugin_interface;
}
