#pragma once

#include "Common/fifo.h"

namespace irtiny
{
    class SerialPort
    {
        SerialPort(SerialPort const&);
        void operator=(SerialPort const&);
    public:
        SerialPort()
            : handle_(INVALID_HANDLE_VALUE)
        { }

        SerialPort(SerialPort&& src)
            : handle_(src.release())
        { }

        explicit SerialPort(std::wstring portName)
            : handle_(INVALID_HANDLE_VALUE)
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

        SerialPort& operator=(SerialPort&& src)
        {
            if (&src != this)
                reset(src.release());

            return *this;
        }

        explicit operator bool() const
        {
            return handle_ != INVALID_HANDLE_VALUE;
        }

        HANDLE release()
        {
            HANDLE res = handle_;
            handle_ = INVALID_HANDLE_VALUE;
            return res;
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

    private:

        HANDLE handle_;
    };

    class Event
    {
        Event(Event const&);
        void operator=(Event const&);

        Event(HANDLE hEvent)
            : handle_(hEvent)
        { }

    public:
        static Event fromHandle(HANDLE hEvent)
        {
            return Event(hEvent);
        }

        static Event manualResetEvent()
        {
            return fromHandle(::CreateEvent(nullptr, TRUE, FALSE, nullptr));
        }

        static Event autoResetEvent()
        {
            return fromHandle(::CreateEvent(nullptr, FALSE, FALSE, nullptr));
        }

        Event(Event&& src)
            : handle_(src.release())
        { }

        ~Event()
        {
            reset();
        }

        Event& operator=(Event&& src)
        {
            if (&src != this)
                reset(src.release());
            return *this;
        }

        explicit operator bool() const
        {
            return handle_ != nullptr;
        }

        HANDLE release()
        {
            HANDLE res = handle_;
            handle_ = nullptr;
            return res;
        }

        HANDLE get() const { return handle_; }

        void reset(HANDLE newHandle = nullptr)
        {
            if (newHandle != handle_)
            {
                if (*this)
                    ::CloseHandle(handle_);
                handle_ = newHandle;
            }
        }

        bool setEvent() const
        {
            return ::SetEvent(handle_) != FALSE;
        }

        bool resetEvent() const
        {
            return ::ResetEvent(handle_) != FALSE;
        }

    private:

        HANDLE handle_;
    };

    class CIRDriver
    {
    public:
        explicit CIRDriver(HANDLE finishEvent);
        ~CIRDriver();

        bool initPort();
        void resetPort();
        std::uint32_t readData(std::chrono::microseconds maxusec);
        bool dataReady() const;
        bool getData(std::uint32_t *out);
        bool waitTillDataIsReady(std::chrono::microseconds maxUSecs);

    private:

        void threadProc();
        void setData(std::uint32_t data);

        SerialPort serialPort_;

        winlirc::Fifo buffer_;
        Event dataReadyEvent_;
        Event finishEvent_;

        std::thread thread_;
    };

}