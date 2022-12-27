#pragma once

#include "UniqueHandle.h"

namespace winlirc
{
using Socket = UniqueHandle<SOCKET, ::closesocket, INVALID_SOCKET>;
}