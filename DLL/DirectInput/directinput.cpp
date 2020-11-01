#define DIRECTINPUT_VERSION 0x0800

#include <Windows.h>
#include "../Common/UniqueHandle.h"
#include <winlirc/WLPluginAPI.h>

#include <atlbase.h> // CComPtr
#include <dinput.h>

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

struct HwndTraits
{
    typedef HWND HandleType;
    constexpr static HandleType invalidValue() noexcept { return nullptr; }
    static void close(HandleType h) noexcept { ::DestroyWindow(h); }
};

using Window = winlirc::UniqueHandle<HwndTraits>;

CComPtr<IDirectInput8> g_di;
CComPtr<IDirectInputDevice8> g_diJoystick;
Window g_window;
HANDLE g_exitEvent = INVALID_HANDLE_VALUE;
bool g_initialized = false;

WL_API int init(WLEventHandle wlExitEvent)
{
    HANDLE const exitEvent = reinterpret_cast<HANDLE>(wlExitEvent);

    if (g_initialized || exitEvent == INVALID_HANDLE_VALUE)
        return 0;

    g_initialized = true;

    auto hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return 0;

    CComPtr<IDirectInput8> di;
    hr = DirectInput8Create(
        GetModuleHandle(nullptr),
        DIRECTINPUT_VERSION,
        IID_IDirectInput8,
        reinterpret_cast<void**>(&di),
        nullptr);
    if (FAILED(hr))
        return 0;

    CComPtr<IDirectInputDevice8> diJoystick;
    hr = di->CreateDevice(
        GUID_Joystick, //GUID_SysKeyboard,
        &diJoystick,
        nullptr);
    if (FAILED(hr))
        return 0;

    hr = diJoystick->SetDataFormat(&c_dfDIJoystick);
    if (FAILED(hr))
        return 0;

    WNDCLASSEX wcx = { 0 };
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = DefWindowProc;
    wcx.hInstance = GetModuleHandle(nullptr);
    wcx.lpszClassName = L"MainWClass";
    RegisterClassEx(&wcx);
    Window wnd{ ::CreateWindow(
        L"MainWClass", L"DirectInputMessages", WS_OVERLAPPEDWINDOW,
        0, 0, 100, 100,
        HWND_MESSAGE, nullptr,
        GetModuleHandle(nullptr),
        nullptr) };

    hr = diJoystick->SetCooperativeLevel(
        wnd.get(),
        DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

    if (FAILED(hr))
        return 0;

    hr = diJoystick->Acquire();
    if (FAILED(hr))
        return 0;

    g_di = di;
    g_window = std::move(wnd);
    g_diJoystick = diJoystick;
    g_exitEvent = exitEvent;
    return 1;
}

WL_API void deinit()
{
    if (g_initialized)
    {
        g_diJoystick.Release();
        g_window.reset();
        g_di.Release();
        g_initialized = false;
        ::CoUninitialize();
    }
}

WL_API int hasGui() { return 0; }

WL_API void loadSetupGui() { }

WL_API int sendIR(struct ir_remote*, struct ir_ncode*, int) { return 0; }

WL_API int decodeIR(struct ir_remote*, char *out, size_t out_size)
{
    if (!g_initialized)
        return 0;

    assert(g_di);
    assert(g_diJoystick);
    assert(g_exitEvent != INVALID_HANDLE_VALUE);

    while (true)
    {
        if (::WaitForSingleObject(g_exitEvent, 50) == WAIT_OBJECT_0)
            return 0;

        DIJOYSTATE state;
        auto hr = g_diJoystick->GetDeviceState(
            sizeof(state),
            &state);
        if (FAILED(hr))
        {
            if (FAILED(g_diJoystick->Acquire()))
                return 0;
        }
        else
        {
            bool foundButton = false;
            std::vector<char> buttonName(out_size);
            char* btn = buttonName.data();
            int bytesLeft = buttonName.size() - 1;
            for (size_t i = 0; i < sizeof(state.rgbButtons); ++i)
            {
                if (state.rgbButtons[i] & 0x80)
                {
                    if (!foundButton)
                    {
                        btn += sprintf(btn, "BUTTON");
                        foundButton = true;
                    }
                    auto const n = sprintf(btn, "_%u", i);
                    if (n > bytesLeft)
                    {
                        foundButton = false;
                        break;
                    }
                    else
                    {
                        bytesLeft -= n;
                        btn += n;
                    }
                }
            }
            // TODO: add support for analogue inputs
            if (foundButton)
            {
                _snprintf_s(out, out_size, out_size, "%016llx %02x %s DirectInput\n", int64_t{0}, 1, buttonName);
                return 1;
            }
        }
    }
}

BOOL WINAPI DllMain(
    _In_  HINSTANCE hinstDLL,
    _In_  DWORD fdwReason,
    _In_  LPVOID lpvReserved)
{
    return TRUE;
}