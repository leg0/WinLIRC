#pragma once

#include <cstdint>

namespace winlirc {

class Fifo
{
public:
    Fifo() noexcept = default;

    void push(std::uint32_t value) noexcept
    {
        dataBuffer[bufferEnd++] = value;
    }

    std::uint32_t pop() noexcept
    {
        return dataBuffer[bufferStart++];
    }

    bool empty() const noexcept
    {
        return bufferStart == bufferEnd;
    }

    void clear() noexcept
    {
        bufferStart = bufferEnd = 0;
    }

private:
    std::uint32_t dataBuffer[256]{};
    std::uint8_t  bufferStart{};
    std::uint8_t  bufferEnd{};
};

} // namespace winlirc
