#include "stdafx.h"
#include "winlirc.h"
#include "drvdlg.h"
#include "server.h"
#include "guicon.h"

#include <tao/json.hpp>
#include <tao/json/contrib/traits.hpp>

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

Cwinlirc app;

static fs::path getModuleFileName()
{
	wchar_t fullPath[MAX_PATH + 1];
	DWORD const pathLength = GetModuleFileNameW(nullptr, fullPath, MAX_PATH);
	return fs::path{ std::wstring_view{fullPath, pathLength} };
}

fs::path getPluginsDirectory()
{
	return absolute(getModuleFileName().replace_filename(L"plugins"));
}

fs::path getConfigsDirectory()
{
	return absolute(getModuleFileName().replace_filename(L"configs"));
}

std::vector<fs::path> listPlugins()
{
	std::vector<fs::path> plugins;
	auto pluginsDir = getPluginsDirectory();
	for (auto& p : fs::directory_iterator(pluginsDir))
	{
		auto path = p.path();
		if (path.extension() == L".dll")
		{
			Plugin plugin{ path };
			if (plugin.hasValidInterface())
				plugins.push_back(absolute(path));
		}
	}
	return plugins;
}

std::vector<fs::path> listConfigs()
{
	std::vector<fs::path> configs;
	auto configsDir = getConfigsDirectory();
	for (auto& p : fs::directory_iterator(configsDir))
	{
		auto path = p.path();
		if (path.extension() == L".conf" || path.extension() == L".cf")
		{
			configs.push_back(absolute(path));
		}
	}
	return configs;
}

static winlirc::WebServer::response get_plugins()
{
	auto plugins = listPlugins();
	std::vector<tao::json::value> pluginNames;
	std::transform(begin(plugins), end(plugins), std::back_inserter(pluginNames), [](auto& path) {
			return path.filename().replace_extension().string();
		});
	tao::json::value val{ { "plugins", pluginNames } };
	winlirc::WebServer::response r{};
	r.body = to_string(val);
	r.mimeType = "text/json";

	return r;
}

static winlirc::WebServer::response get_settings()
{
	tao::json::value val{ {"settings", {
			{"config", config.remoteConfig.filename().replace_extension().string()},
			{"plugin", config.plugin.filename().replace_extension().string()},
			{"disable_repeats", config.disableRepeats != 0},
			{"disable_first_repeats", config.disableFirstKeyRepeats},
			{"server_port", config.serverPort},
			{"local_connections_only", config.localConnectionsOnly != 0},
			{"show_tray_icon", config.showTrayIcon != 0},
			{"exit_on_error", config.exitOnError != 0}
		} } };

	winlirc::WebServer::response r{};
	r.body = to_string(val);
	r.mimeType = "text/json";
	return r;
}

static winlirc::WebServer::response put_settings(winlirc::IHttpMessage const& request)
{
	auto body = request.body();

	auto j = tao::json::from_string(body);
	return { };
}

static winlirc::WebServer::response get_configs()
{
	auto configs = listConfigs();
	std::vector<tao::json::value> configNames;
	std::transform(begin(configs), end(configs), std::back_inserter(configNames), [](auto& path) {
			return path.filename().replace_extension().string();
		});
	tao::json::value val{ { "configs", configNames } };

	winlirc::WebServer::response r{};
	r.body = to_string(val);
	r.mimeType = "text/json";
	return r;
}

BOOL Cwinlirc::InitInstance() {

	AfxInitRichEdit();

#ifdef _DEBUG
	RedirectIOToConsole();
#endif

	// set current directory for plugins from exe path
	fs::current_path(getPluginsDirectory());

	config.readINIFile();

	//
	// command line stuff
	//

	std::wstring_view cmdLine{ m_lpCmdLine };
	auto contains = [](std::wstring_view s, std::wstring_view b) {
		return s.find(b) != std::wstring_view::npos;
	};
	config.exitOnError = contains(cmdLine, L"/e") || contains(cmdLine, L"/E");
	config.showTrayIcon = !(contains(cmdLine, L"/t") || contains(cmdLine, L"/T"));

	wchar_t mutexName[64];
	wsprintf(mutexName, L"WinLIRC Multiple Instance Lockout_%i", config.serverPort);

	if (!CreateMutex(nullptr, FALSE, mutexName) || GetLastError()==ERROR_ALREADY_EXISTS)
	{
		HWND const winlirc = FindWindow(nullptr,_T("WinLIRC"));
		if (!winlirc)
		{
			MessageBoxW(nullptr, L"WinLIRC is already running", L"WinLIRC", MB_OK);
		}
		else
		{
			// bring it to the top
			HWND const last = GetLastActivePopup(winlirc);
			if (!IsWindowVisible(winlirc))
				ShowWindow(winlirc, SW_SHOW);

			SetForegroundWindow(winlirc);
			SetForegroundWindow(last);
		}
		return FALSE;
	}

	//
	//Process initialization and sanity checks
	//
	if(SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS)==0 || SetThreadPriority(THREAD_PRIORITY_IDLE)==0) {
		MessageBoxW(nullptr, L"Could not set thread priority.", L"WinLIRC", MB_OK|MB_ICONERROR);
		return FALSE;
	}
	
	if(!server.startServer()) {
		MessageBoxW(nullptr,L"Server could not be started. Try checking the port.", L"WinLIRC", MB_OK|MB_ICONERROR);
	}

	WL_DEBUG("Creating main dialog...\n");

	dlg.reset(new Cdrvdlg());

	if(!dlg->Create(IDD_DIALOG,nullptr)) {
		dlg.reset();
		MessageBoxW(nullptr, L"Program exiting.", L"WinLIRC", MB_OK|MB_ICONERROR);
		return FALSE;
	}

	dlg->ShowWindow(SW_HIDE);
	dlg->UpdateWindow();
	m_pMainWnd = dlg.get();
	
	webServer = std::make_unique<winlirc::WebServer>(8766);
	webServer->RegisterEndpoint(
		"GET",
		"/api/v1/plugins",
		[](winlirc::svmatch const&, winlirc::IHttpMessagePtr) { return get_plugins(); });
	webServer->RegisterEndpoint(
		"GET",
		"/api/v1/configs",
		[](winlirc::svmatch const&, winlirc::IHttpMessagePtr) { return get_configs(); });
	webServer->RegisterEndpoint(
		"GET",
		"/api/v1/settings",
		[](winlirc::svmatch const&, winlirc::IHttpMessagePtr) { return get_settings(); });
	webServer->RegisterEndpoint(
		"PUT",
		"/api/v1/settings",
		[](winlirc::svmatch const&, winlirc::IHttpMessagePtr request) { return put_settings(*request); });
	webServer->Start();

	return TRUE;
}

int Cwinlirc::ExitInstance()
{
	dlg.reset();
	webServer.reset();
	return CWinApp::ExitInstance();
}
