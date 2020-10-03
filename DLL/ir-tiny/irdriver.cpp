#include "stdafx.h"
#include "irdriver.h"
#include "Settings.h"


irtiny::CIRDriver::CIRDriver(HANDLE finishEvent)
    : dataReadyEvent_(Event::manualResetEvent())
    , finishEvent_(Event::fromHandle(finishEvent))
{ }

irtiny::CIRDriver::~CIRDriver()
{
    if (thread_.joinable())
        thread_.join();

    // we're not owning that.
    finishEvent_.release();
}

bool irtiny::CIRDriver::initPort()
{
    buffer_.clear();

    if (thread_.joinable())
    {
        finishEvent_.setEvent();
        thread_.join();
    }

    Settings settings;
    settings.load();

    SerialPort serialPort = SerialPort(settings.port());
    if (!serialPort)
        return false;

    DCB dcb = { 0 };

    // The device is powered by RTS
    // and running at fixed baud rate.

    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = CBR_115200;
    dcb.fBinary = TRUE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(serialPort.get(), &dcb))
        return false;

    PurgeComm(serialPort.get(), PURGE_RXABORT | PURGE_RXCLEAR);
    ClearCommError(serialPort.get(), nullptr, nullptr);

    serialPort_ = std::move(serialPort);
    thread_ = std::thread([=]() { threadProc(); });

    return true;
}

void irtiny::CIRDriver::threadProc()
{
    Event overlappedEvent = Event::manualResetEvent();
    OVERLAPPED ov = { 0 };
    ov.hEvent = overlappedEvent.get();

    HANDLE const events[2] = { ov.hEvent, finishEvent_.get() };

    // Send a byte to force RTS
    ::WriteFile(serialPort_.get(), "x", 1, nullptr, &ov);

    ::Sleep(100);

    for (;;)
    {
        overlappedEvent.resetEvent();

        // We want to be notified of RX changes
        if (SetCommMask(serialPort_.get(), EV_RXCHAR) == 0)
        {
            //DEBUG("SetCommMask returned zero, error=%d\n",GetLastError());
        }
        // Start waiting for the event
        DWORD event;
        if (WaitCommEvent(serialPort_.get(), &event, &ov) == 0 && GetLastError() != 997)
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
                auto const readSuccessful = ReadFile(serialPort_.get(), buf, 2, &bytesRead, &ov);
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
                    ::ClearCommError(serialPort_.get(), nullptr, nullptr);
                    ::PurgeComm(serialPort_.get(), PURGE_RXABORT | PURGE_RXCLEAR);
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

