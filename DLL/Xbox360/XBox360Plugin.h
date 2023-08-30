#pragma once

#include <Windows.h>
#include <XInput.h>
#include "../Common/Event.h"
#include <winlirc/WLPluginAPI.h>

#include <chrono>
#include <thread>
#include <mutex>

class XBox360Plugin : public plugin_interface {
public:
    XBox360Plugin() noexcept;
    ~XBox360Plugin() noexcept;

    void init(HANDLE exit) noexcept;
    bool waitTillDataIsReady(std::chrono::microseconds maxUSecs) const;
    bool dataReady() const;
    int	 decodeCommand(char* out, size_t out_size);

private:
    void threadProc(std::stop_token t);

    XINPUT_STATE    m_controllerState;
    std::jthread    m_threadHandle;
    HANDLE const    m_threadExitEvent; // not owned
    winlirc::Event  m_dataReadyEvent;
    
    mutable std::mutex m_stateMutex;
    int m_value = 0;
    int m_repeats = 0;
};
