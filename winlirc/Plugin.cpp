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
		else
		{
			interface_.init = (InitFunction)GetProcAddress(d.get(), "init");
			interface_.deinit = (DeinitFunction)GetProcAddress(d.get(), "deinit");
			interface_.hasGui = (HasGuiFunction)GetProcAddress(d.get(), "hasGui");
			interface_.loadSetupGui = (LoadSetupGuiFunction)GetProcAddress(d.get(), "loadSetupGui");
			interface_.sendIR = (SendFunction)GetProcAddress(d.get(), "sendIR");
			interface_.decodeIR = (DecodeFunction)GetProcAddress(d.get(), "decodeIR");
			interface_.setTransmitters = (SetTransmittersFunction)GetProcAddress(d.get(), "setTransmitters");
			interface_.getHardware = (GethardwareFunction)GetProcAddress(d.get(), "getHardware");
		}
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
