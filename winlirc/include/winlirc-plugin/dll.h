#pragma once

#include <winlirc-common/UniqueHandle.h>

struct DllTraits
{
	using HandleType = HMODULE;
	static constexpr HandleType invalidValue() noexcept
	{
		return nullptr;
	}

	static void close(HandleType h) noexcept
	{
		::FreeLibrary(h);
	}
};

using Dll = winlirc::UniqueHandle<DllTraits>;
