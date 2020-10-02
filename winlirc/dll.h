#pragma once

#include "../DLL/Common/UniqueHandle.h"

struct DllTraits
{
    using HandleType = HMODULE;
    static HandleType invalidValue() noexcept { return nullptr; }
    static void close(HandleType h) noexcept { if (h != invalidValue()) FreeLibrary(h); }
};

using Dll = winlirc::UniqueHandle<DllTraits>;