#pragma once

#include "../DLL/Common/UniqueHandle.h"

namespace winlirc
{
    using Dll = UniqueHandle<HMODULE, ::FreeLibrary>;
}
