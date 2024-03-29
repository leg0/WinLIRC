#pragma once

#include <winlirc/WLPluginAPI.h>
#include "dll.h"
#include <filesystem>
#include <string>

using GetPluginInterfaceFunction = decltype(::getPluginInterface)*;

struct Plugin
{
	Plugin() = default;
	Plugin(Plugin const&) = delete;
	Plugin(Plugin&&) = default;
	Plugin(wchar_t const* path) = delete;
	explicit Plugin(std::wstring const& path) noexcept;
	explicit Plugin(std::filesystem::path const& path) noexcept;
	Plugin& operator=(Plugin const&) = delete;
	Plugin& operator=(Plugin&&) = default;

	explicit operator bool() const noexcept { return static_cast<bool>(dllFile_); }

	bool hasValidInterface() const noexcept;
	bool canRecord() const noexcept;

	plugin_interface interface_ = { 0 };
	Dll dllFile_;
};
