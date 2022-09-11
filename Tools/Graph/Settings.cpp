#include  <Windows.h>
#include "Settings.h"
#include <filesystem>

namespace fs = std::filesystem;

std::optional<std::wstring> Settings::getPluginName()
{
	auto const path = fs::current_path() / L"WinLIRC.ini";

	wchar_t pluginName[128];
	auto const count = GetPrivateProfileStringW(L"WinLIRC", L"Plugin", nullptr, pluginName, std::size(pluginName), path.c_str());

	if (count > 0)
		return pluginName;
	else
		return {};
}
