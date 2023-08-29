#include <Windows.h>
#include "Plugin.h"
#include <cassert>

Plugin::Plugin(std::wstring const& path) noexcept
	: dllFile_ { LoadLibraryW(path.c_str()) }
{
	if (Dll& d = dllFile_)
	{
		auto getPluginInterface = (GetPluginInterfaceFunction)GetProcAddress(d.get(), "getPluginInterface");
		if (getPluginInterface)
			interface_ = *getPluginInterface();
	}
}

Plugin::Plugin(std::filesystem::path const& path) noexcept
	: Plugin{ path.wstring() }
{ }

bool Plugin::hasValidInterface() const noexcept
{
	if (!*this)
		return false;

	auto& i = interface_;
	if (i.init && i.deinit && i.hasGui && i.decodeIR)
		return !i.hasGui() || i.loadSetupGui;
	else
		return false;
}

bool Plugin::canRecord() const noexcept
{
	return *this && (interface_.hardware || interface_.getHardware);
}
