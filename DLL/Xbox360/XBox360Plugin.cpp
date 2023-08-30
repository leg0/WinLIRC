#include "XBox360Plugin.h"


#include <Windows.h>
#include <winlirc/WLPluginAPI.h>

using namespace std::chrono_literals;

static int xbox360_init(plugin_interface* self, winlirc_api const* winlirc) {

    auto const sendReceive = static_cast<XBox360Plugin*>(self);
    sendReceive->init(reinterpret_cast<HANDLE>(winlirc->getExitEvent(winlirc)));
    return 1;
}

static void xbox360_deinit(plugin_interface* self) {
    delete self;
}

static int xbox360_decodeIR(plugin_interface* self, ir_remote* remotes, char* out, size_t out_size) {
    auto const sendReceive = static_cast<XBox360Plugin*>(self);
    if (sendReceive) {
        if (!sendReceive->waitTillDataIsReady(0us)) {
            return 0;
        }

        return sendReceive->decodeCommand(out, out_size);
    }

    return 0;
}

XBox360Plugin::XBox360Plugin() noexcept
    : plugin_interface {
        .plugin_api_version = winlirc_plugin_api_version,
        .init = xbox360_init,
        .deinit = xbox360_deinit,
        .hasGui = [](plugin_interface*) { return FALSE; },
        .loadSetupGui = [](plugin_interface*) {},
        .sendIR = [](auto, auto, auto, auto) { return 0; },
        .decodeIR = xbox360_decodeIR

    }
    , m_dataReadyEvent{ winlirc::Event::manualResetEvent() }
    , m_threadExitEvent{ exit }
{ }

void XBox360Plugin::init(HANDLE exit) noexcept {
    m_threadHandle = std::jthread{ [this](std::stop_token st) { threadProc(st); } };
}

XBox360Plugin::~XBox360Plugin() noexcept {
    m_threadHandle.request_stop();
}

void XBox360Plugin::threadProc(std::stop_token stop) {

    {
        std::scoped_lock l{ m_stateMutex };
        m_value = 0;
        m_repeats = 0;
    }

    while (!stop.stop_requested()) {

        XINPUT_KEYSTROKE keyStroke{};
        auto const result = XInputGetKeystroke(XUSER_INDEX_ANY, 0, &keyStroke);
        if (result == ERROR_SUCCESS) {

            if (keyStroke.Flags & XINPUT_KEYSTROKE_KEYDOWN) {

                bool const isRepeat = keyStroke.Flags & XINPUT_KEYSTROKE_REPEAT;
                {
                    std::scoped_lock l{ m_stateMutex };
                    m_value = keyStroke.VirtualKey;
                    m_repeats = isRepeat ? (m_repeats + 1) : 0;
                }

                m_dataReadyEvent.setEvent();
            }
        }
        else if (result == ERROR_EMPTY) {
            Sleep(10);
        }
        else if (result == ERROR_DEVICE_NOT_CONNECTED) {
            Sleep(1000);
        }
    }
}

bool XBox360Plugin::waitTillDataIsReady(std::chrono::microseconds maxUSecs) const {

    HANDLE const events[] = { m_dataReadyEvent.get(), m_threadExitEvent };
    DWORD const count = (m_threadExitEvent == nullptr) ? 1 : 2;

    if (!dataReady()) {

        m_dataReadyEvent.resetEvent();

        using namespace std::chrono;
        DWORD const dwTimeout = maxUSecs > 0us
            ? duration_cast<milliseconds>(maxUSecs + 500us).count()
            : INFINITE;
        DWORD const result = ::WaitForMultipleObjects(count, events, false, dwTimeout);

        if (result == (WAIT_OBJECT_0 + 1)) {
            return false;
        }
    }

    return true;
}

bool XBox360Plugin::dataReady() const {
    std::scoped_lock l{ m_stateMutex };
    return m_value != 0;
}

int XBox360Plugin::decodeCommand(char* out, size_t out_size) {

    auto const [value, repeats] = [this]() {
        std::scoped_lock l{ m_stateMutex };
        return std::pair{ std::exchange(m_value, 0), m_repeats };
    }();
    auto const buttonName = [](int value) {
        switch (value) {
        case VK_PAD_A:					return "A";
        case VK_PAD_B:					return "B";
        case VK_PAD_X:					return "X";
        case VK_PAD_Y:					return "Y";
        case VK_PAD_RSHOULDER:			return "RSHOULDER";
        case VK_PAD_LSHOULDER:			return "LSHOULDER";
        case VK_PAD_LTRIGGER:			return "LTRIGGER";
        case VK_PAD_RTRIGGER:			return "RTRIGGER";
        case VK_PAD_DPAD_UP:			return "DPAD_UP";
        case VK_PAD_DPAD_DOWN:			return "DPAD_DOWN";
        case VK_PAD_DPAD_LEFT:			return "DPAD_LEFT";
        case VK_PAD_DPAD_RIGHT:			return "DPAD_RIGHT";
        case VK_PAD_START:				return "START";
        case VK_PAD_BACK:				return "BACK";
        case VK_PAD_LTHUMB_PRESS:		return "LTHUMB_PRESS";
        case VK_PAD_RTHUMB_PRESS:		return "RTHUMB_PRESS";
        case VK_PAD_LTHUMB_UP:			return "LTHUMB_UP";
        case VK_PAD_LTHUMB_DOWN:		return "LTHUMB_DOWN";
        case VK_PAD_LTHUMB_RIGHT:		return "LTHUMB_RIGHT";
        case VK_PAD_LTHUMB_LEFT:		return "LTHUMB_LEFT";
        case VK_PAD_LTHUMB_UPLEFT:		return "LTHUMB_UPLEFT";
        case VK_PAD_LTHUMB_UPRIGHT:		return "LTHUMB_UPRIGHT";
        case VK_PAD_LTHUMB_DOWNRIGHT:	return "LTHUMB_DOWNRIGHT";
        case VK_PAD_LTHUMB_DOWNLEFT:	return "LTHUMB_DOWNLEFT";
        case VK_PAD_RTHUMB_UP:			return "RTHUMB_UP";
        case VK_PAD_RTHUMB_DOWN:		return "RTHUMB_DOWN";
        case VK_PAD_RTHUMB_RIGHT:		return "RTHUMB_RIGHT";
        case VK_PAD_RTHUMB_LEFT:		return "RTHUMB_LEFT";
        case VK_PAD_RTHUMB_UPLEFT:		return "RTHUMB_UPLEFT";
        case VK_PAD_RTHUMB_UPRIGHT:		return "RTHUMB_UPRIGHT";
        case VK_PAD_RTHUMB_DOWNRIGHT:	return "RTHUMB_DOWNRIGHT";
        case VK_PAD_RTHUMB_DOWNLEFT:	return "RTHUMB_DOWNLEFT";
        default: return static_cast<char const*>(nullptr);
        }
    }(m_value);
    if (buttonName == nullptr) {
        return 0;
    }
    else {
        _snprintf_s(out, out_size, out_size, "%016llx %02x %s Xbox360\n", int64_t{}, repeats, buttonName);
        return 1;
    }
}

WL_API plugin_interface* getPluginInterface()
{
    return new XBox360Plugin();
}
