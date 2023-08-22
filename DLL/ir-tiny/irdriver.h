#pragma once

#include "Common/Event.h"
#include "Common/fifo.h"

namespace irtiny
{
    class SerialPort
    {
    public:
        SerialPort(SerialPort const&) = delete;
        SerialPort& operator=(SerialPort const&) = delete;

        SerialPort() noexcept = default;

        SerialPort(SerialPort&& src) noexcept
            : handle_(src.release())
        { }

        explicit SerialPort(std::wstring const& portName) noexcept
        {
            handle_ = CreateFile(
                portName.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                0,
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                0);
        }

        ~SerialPort()
        {
            reset();
        }

        SerialPort& operator=(SerialPort&& src) noexcept
        {
            if (&src != this)
                reset(src.release());

            return *this;
        }

        explicit operator bool() const noexcept
        {
            return handle_ != INVALID_HANDLE_VALUE;
        }

        HANDLE release() noexcept
        {
            return std::exchange(handle_, INVALID_HANDLE_VALUE);
        }

        HANDLE get() const { return handle_; }

        void reset(HANDLE newHandle = INVALID_HANDLE_VALUE)
        {
            if (newHandle != handle_)
            {
                if (*this)
                    ::CloseHandle(handle_);
                handle_ = newHandle;
            }
        }

        friend auto SetCommState(SerialPort const& p, DCB const& dcb) noexcept
        {
            return ::SetCommState(p.handle_, const_cast<DCB*>(&dcb));
        }

        friend auto SetCommMask(SerialPort const& p, DWORD evtMask) noexcept
        {
            return ::SetCommMask(p.handle_, evtMask);
        }

        friend auto WaitCommEvent(SerialPort const& p, DWORD& evtMask, OVERLAPPED& overlapped) noexcept
        {
            return ::WaitCommEvent(p.handle_, &evtMask, &overlapped);
        }

        friend auto PurgeComm(SerialPort const& p, DWORD flags) noexcept
        {
            return ::PurgeComm(p.handle_, flags);
        }

        friend auto ClearCommError(SerialPort const& p, LPDWORD errors = nullptr, LPCOMSTAT stat = nullptr) noexcept
        {
            return ::ClearCommError(p.handle_, errors, stat);
        }

        friend auto ReadFile(SerialPort const& p, LPVOID lpBuffer, DWORD numberOfBytesToRead, DWORD& numberOfBytesRead, OVERLAPPED& overlapped) noexcept
        {
            return ::ReadFile(p.handle_, lpBuffer, numberOfBytesToRead, &numberOfBytesRead, &overlapped);
        }

        friend auto WriteFile(SerialPort const& p, LPCVOID lpBuffer, DWORD numberOfBytesToWrite, DWORD& numberOfBytesWritten, OVERLAPPED& overlapped) noexcept
        {
            return ::WriteFile(p.handle_, lpBuffer, numberOfBytesToWrite, &numberOfBytesWritten, &overlapped);
        }

    private:

        HANDLE handle_{ INVALID_HANDLE_VALUE };
    };

    using Event = winlirc::Event;

    class CIRDriver
    {
    public:
        explicit CIRDriver(HANDLE finishEvent, std::wstring const& port);
        ~CIRDriver();

        bool initPort();
        std::uint32_t readData(std::chrono::microseconds maxusec);
        bool dataReady() const;
        bool waitTillDataIsReady(std::chrono::microseconds maxUSecs);

    private:

        void threadProc(std::stop_token stop);
        void setData(std::uint32_t data);

        SerialPort serialPort_;

        winlirc::Fifo buffer_;
        Event dataReadyEvent_;
        Event finishEvent_;

        std::jthread thread_;
    };

}