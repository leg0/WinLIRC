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

    if (serialPort_)
    {
        // Stop any waiting on the port.
        SetCommMask(serialPort_.get(), 0);
        Sleep(100);
        serialPort_.reset();
    }

    Settings settings;
    settings.load();

    SerialPort serialPort = SerialPort(settings.port());
    if (!serialPort)
        return false;

    DCB dcb;
    if (!GetCommState(serialPort.get(), &dcb))
        return false;

    // The device is powered by RTS
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    // and running at fixed baud rate.
    dcb.BaudRate = 115200;

    if (!SetCommState(serialPort.get(), &dcb))
        return false;

    thread_ = std::thread([=]() { this->threadProc(); });

    serialPort_ = std::move(serialPort);
    return true;
}

void irtiny::CIRDriver::resetPort()
{
    serialPort_.reset();
}

void irtiny::CIRDriver::threadProc()
{
    Event overlappedEvent = Event::manualResetEvent();
    OVERLAPPED ov = { 0 };
    ov.hEvent = overlappedEvent.get();

    HANDLE const events[2] = { ov.hEvent, finishEvent_.get() };
    ::Sleep(100);
    COMMTIMEOUTS commTimeouts = { 0 };
    commTimeouts.ReadIntervalTimeout = ~0;
    ::SetCommTimeouts(serialPort_.get(), &commTimeouts);
    ::PurgeComm(serialPort_.get(), PURGE_RXABORT | PURGE_RXCLEAR);

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

std::uint32_t irtiny::CIRDriver::readData(std::uint32_t maxusec)
{
    if (waitTillDataIsReady(maxusec))
        return buffer_.pop();
    else
        return 0;
}

bool irtiny::CIRDriver::waitTillDataIsReady(std::uint32_t maxUSecs)
{
    if (dataReady())
    {
        return true;
    }
    else
    {
        dataReadyEvent_.resetEvent();
        HANDLE const events[2] = { dataReadyEvent_.get(), finishEvent_.get() };
        DWORD const dwTimeout = maxUSecs
            ? (maxUSecs + 500) / 1000
            : INFINITE;
        DWORD const res = ::WaitForMultipleObjects(2, events, false, dwTimeout);
        return res == WAIT_OBJECT_0;
    }
}

