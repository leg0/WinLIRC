#pragma once

#include <cstdint>

namespace winlirc {

class Fifo
{
public:
    Fifo()
        : bufferStart(0)
        , bufferEnd(0)
    { }

    void push(std::uint32_t value)
    {
        dataBuffer[bufferEnd++] = value;
    }

    std::uint32_t pop()
    {
        return dataBuffer[bufferStart++];
    }

    bool empty() const
    {
        return bufferStart == bufferEnd;
    }

    void clear()
    {
        bufferStart = bufferEnd = 0;
    }

private:
    std::uint32_t dataBuffer[256];
    std::uint8_t  bufferStart;
    std::uint8_t  bufferEnd;
};

} // namespace winlirc
