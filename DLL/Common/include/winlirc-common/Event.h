#pragma once

namespace winlirc
{
    class Event
    {
        explicit Event(HANDLE hEvent) noexcept
            : handle_(hEvent)
        { }

    public:
        static Event fromHandle(HANDLE hEvent) noexcept
        {
            return Event(hEvent);
        }

        static Event manualResetEvent() noexcept
        {
            return fromHandle(::CreateEvent(nullptr, true, false, nullptr));
        }

        static Event autoResetEvent() noexcept
        {
            return fromHandle(::CreateEvent(nullptr, false, false, nullptr));
        }

        Event() noexcept
            : handle_(autoResetEvent().release())
        { }

        Event(Event const&) = delete;

        Event(Event&& src) noexcept
            : handle_(src.release())
        { }

        ~Event() noexcept
        {
            reset();
        }

        Event& operator=(Event const&) = delete;

        Event& operator=(Event&& src) noexcept
        {
            if (&src != this)
                reset(src.release());
            return *this;
        }

        explicit operator bool() const noexcept
        {
            return handle_ != nullptr;
        }

        HANDLE release() noexcept
        {
            HANDLE res = handle_;
            handle_ = nullptr;
            return res;
        }

        HANDLE get() const noexcept { return handle_; }

        void reset(HANDLE newHandle = nullptr) noexcept
        {
            if (newHandle != handle_)
            {
                if (*this)
                    ::CloseHandle(handle_);
                handle_ = newHandle;
            }
        }

        bool setEvent() const noexcept
        {
            return ::SetEvent(handle_);
        }

        bool resetEvent() const noexcept
        {
            return ::ResetEvent(handle_);
        }

    private:

        HANDLE handle_;
    };

}
