#define DIRECTINPUT_VERSION 0x0800

#include <atlbase.h> // CComPtr
#include <dinput.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <utility>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

struct Window
{
    explicit Window(HWND h = nullptr) :hWnd(h) { }
    Window(Window const&) = delete;
    void operator=(Window const&) = delete;
    Window(Window&& src) : hWnd(src.release()) { }
    Window& operator=(Window&& src)
    {
        if (this != &src)
        {
            close();
            hWnd = src.release();
        }
        return *this;
    }
    ~Window() { close(); }

    HWND release()
    {
        auto const res = hWnd;
        hWnd = nullptr;
        return res;
    }

    void close()
    {
        if (hWnd)
        {
            ::DestroyWindow(hWnd);
            hWnd = nullptr;
        }
    }

    HWND get() const { return hWnd; }

private:
    HWND hWnd;
};


CComPtr<IDirectInput8> g_di;
CComPtr<IDirectInputDevice8> g_diJoystick;
Window g_window;
HANDLE g_exitEvent = INVALID_HANDLE_VALUE;

DWORD g_initThreadId = 0;
extern "C" int init(HANDLE exitEvent)
{
    assert(exitEvent != INVALID_HANDLE_VALUE);
    assert(g_initThreadId == 0);
    g_initThreadId = ::GetCurrentThreadId();

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
        L"MainWClass", L"Sample", WS_OVERLAPPEDWINDOW,
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

extern "C" void deinit()
{
    if (g_initThreadId != 0)
    {
        g_diJoystick.Release();
        g_window.close();
        g_di.Release();
        g_initThreadId = 0;
        ::CoUninitialize();
    }
}

extern "C" int hasGui() { return 0; }

extern "C" void loadSetupGui() { }

extern "C" int sendIR(struct ir_remote*, struct ir_ncode*, int) { return 0; }

extern "C" int decodeIR(struct ir_remote*, char *out)
{
    //assert(::GetCurrentThreadId() == g_initThreadId);
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
            size_t const PACKET_SIZE = 256;
            char buttonName[PACKET_SIZE-40];
            char* btn = buttonName;
            auto bytesLeft = sizeof(buttonName) - 1;
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
                _snprintf_s(out, PACKET_SIZE, PACKET_SIZE, "%016llx %02x %s DirectInput\n", int64_t{0}, 1, buttonName);
                return 1;
            }
        }
    }
}

extern "C" struct hardware* getHardware()
{
    return nullptr;
}

BOOL WINAPI DllMain(
    _In_  HINSTANCE hinstDLL,
    _In_  DWORD fdwReason,
    _In_  LPVOID lpvReserved)
{
    return TRUE;
}