#pragma once

#include "UniqueHandle.h"

namespace winlirc
{
struct SocketTraits
{
	using HandleType = SOCKET;

	static HandleType invalidValue() noexcept
	{
		return INVALID_SOCKET;
	}

	static void close(HandleType h) noexcept
	{
		if (h != invalidValue())
			::closesocket(h);
	}
};

using Socket = UniqueHandle<SocketTraits>;
}