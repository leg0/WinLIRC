#pragma once

#include <winlirc/WLPluginAPI.h>
#include "dll.h"
#include <filesystem>
#include <string>

using GetPluginInterfaceFunction = decltype(::getPluginInterface)*;
using InitFunction = decltype(::init)*;
using DeinitFunction = decltype(::deinit)*;
using HasGuiFunction = decltype(::hasGui)*;
using LoadSetupGuiFunction = decltype(::loadSetupGui)*;
using SendFunction = decltype(::sendIR)*;
using DecodeFunction = decltype(::decodeIR)*;
using SetTransmittersFunction = decltype(::setTransmitters)*;
using GethardwareFunction = decltype(::getHardware)*;

struct Plugin
{
	Plugin() = default;
	Plugin(Plugin const&) = delete;
	Plugin(Plugin&&) = default;
	explicit Plugin(wchar_t const* path) noexcept;
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
