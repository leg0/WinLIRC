#include "stdafx.h"
#include "irdriver.h"

#include <cassert>

irtiny::CIRDriver::CIRDriver(HANDLE finishEvent, std::wstring const& port)
    : serialPort_{ port }
    , dataReadyEvent_{ Event::manualResetEvent() }
    , finishEvent_{ Event::fromHandle(finishEvent) }
{ }

irtiny::CIRDriver::~CIRDriver()
{
    if (thread_.joinable()) {
        thread_.request_stop();
        finishEvent_.setEvent();
        thread_.join();
    }

    // we're not owning that.
    finishEvent_.release();
}

bool irtiny::CIRDriver::initPort()
{
    assert(!thread_.joinable());
    if (thread_.joinable())
        return false;

    buffer_.clear();

    if (!serialPort_)
        return false;

    // The device is powered by RTS
    // and running at fixed baud rate.

    DCB const dcb = {
        .DCBlength = sizeof(dcb),
        .BaudRate = CBR_115200,
        .fBinary = TRUE,
        .fRtsControl = RTS_CONTROL_ENABLE,
        .ByteSize = 8,
        .Parity = NOPARITY,
        .StopBits = ONESTOPBIT,
    };

    if (!SetCommState(serialPort_, dcb))
        return false;

    PurgeComm(serialPort_, PURGE_RXABORT | PURGE_RXCLEAR);
    ClearCommError(serialPort_);

    thread_ = std::jthread([=](std::stop_token t) { threadProc(t); });

    return true;
}

void irtiny::CIRDriver::threadProc(std::stop_token stop)
{
    Event overlappedEvent = Event::manualResetEvent();
    OVERLAPPED ov = { 0 };
    ov.hEvent = overlappedEvent.get();

    HANDLE const events[2] = { ov.hEvent, finishEvent_.get() };

    // Send a byte to force RTS
    DWORD nBytesWritten{};
    WriteFile(serialPort_, "x", 1, nBytesWritten, ov);

    ::Sleep(100);

    while (!stop.stop_requested())
    {
        overlappedEvent.resetEvent();

        // We want to be notified of RX changes
        if (SetCommMask(serialPort_, EV_RXCHAR) == 0)
        {
            //DEBUG("SetCommMask returned zero, error=%d\n",GetLastError());
        }
        // Start waiting for the event
        DWORD event{};
        if (WaitCommEvent(serialPort_, event, ov) == 0 && GetLastError() != 997)
        {
            //DEBUG("WaitCommEvent error: %d\n",GetLastError());
        }

        switch (WaitForMultipleObjects(2, events, FALSE, INFINITE))
        {
        default:
            continue;

        case WAIT_OBJECT_0:
            while (true)
            {
                uint8_t buf[2];
                DWORD bytesRead = 0;
                auto const readSuccessful = ReadFile(serialPort_, buf, 2, bytesRead, ov);
                if (bytesRead == 2)
                {
                    uint16_t const val = MAKEWORD(buf[1], buf[0]) & 0x7FFF;
                    bool const level = (buf[0] & 0x80) != 0;
                    double const unit = 1.0 / 115200.0;
                    DWORD const deltv = static_cast<DWORD>(1.0e+6 * unit * double(val));
                    DWORD const data = level
                        ? (deltv | 0x1000000)
                        : deltv;
                    setData(data);
                }
                else
                {
                    ClearCommError(serialPort_);
                    PurgeComm(serialPort_, PURGE_RXABORT | PURGE_RXCLEAR);
                    break;
                }
            }
            break;

        case WAIT_OBJECT_0 + 1:
            return;
        }
    }
}

void irtiny::CIRDriver::setData(std::uint32_t data)
{
    buffer_.push(data);
    dataReadyEvent_.setEvent();
}

bool irtiny::CIRDriver::dataReady() const
{
    return !buffer_.empty();
}

std::uint32_t irtiny::CIRDriver::readData(std::chrono::microseconds maxusec)
{
    if (waitTillDataIsReady(maxusec))
        return buffer_.pop();
    else
        return 0;
}

bool irtiny::CIRDriver::waitTillDataIsReady(std::chrono::microseconds maxUSecs)
{
    if (dataReady())
    {
        return true;
    }
    else
    {
        dataReadyEvent_.resetEvent();
        HANDLE const events[2] = { dataReadyEvent_.get(), finishEvent_.get() };
        using namespace std::chrono;
        DWORD const dwTimeout = maxUSecs > 0us
            ? duration_cast<milliseconds>(maxUSecs + 500us).count()
            : INFINITE;
        DWORD const res = ::WaitForMultipleObjects(2, events, false, dwTimeout);
        return res == WAIT_OBJECT_0;
    }
}

