#include "stdafx.h"
#include "ConfigDialog.h"
#include "irdriver.h"
#include "../DLL/Common/enumSerialPorts.h"
#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include <filesystem>

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

extern "C" size_t irtiny_getSetupParameters(char* out, size_t out_size)
{
    std::string setupParameters = R"([{"label":"Serial port", "name": "port", "type":"select", "selectOptions":[)";
    char const* sep = "";
    enumSerialPorts<char>([&](char const* s) {
        setupParameters.append(sep).append(R"__({"name":")__").append(s).append(R"__(","value":"\\\\.\\)__").append(s).append("\"}");
        sep = ",";
    });
    setupParameters += "]}]";
    if (setupParameters.size() <= out_size)
        memcpy(out, setupParameters.data(), setupParameters.size());
    return setupParameters.size();
}


extern "C" size_t irtiny_getSetup(char* out, size_t out_size)
{
    wchar_t fn[MAX_PATH];
    DWORD fnSize= GetModuleFileNameW(nullptr, fn, std::size(fn));
    auto iniFile = std::filesystem::path { fn, fn + fnSize }.remove_filename().append(L"WinLIRC.ini");
    char portName[64];
    GetPrivateProfileStringA("SerialDevice", "Port", R"(\\.\COM1)", portName, std::size(portName) - 1, iniFile.string().c_str());
    std::string setup = R"__({"name":"port", "value":")__";
    setup.append(portName).append(R"__("})__");

    if (setup.size() <= out_size)
        memcpy(out, setup.data(), setup.size());
    return setup.size();
}

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
    .hardware           = &irtiny_hardware,
    .getSetupParameters = irtiny_getSetupParameters,
    .getSetup           = irtiny_getSetup,
};

WL_API plugin_interface const* getPluginInterface()
{
    return &irtiny_plugin_interface;
}
