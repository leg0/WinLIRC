#pragma once

#include "irdriver.h"
#include <sal.h>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

class CIRConfig
{
public:
	explicit CIRConfig(std::filesystem::path iniFilePath);
	~CIRConfig();

	bool readConfig	();
	bool writeINIFile() const;
	bool readINIFile();

	std::wstring readString(
		_In_opt_z_ wchar_t const* section,
		_In_opt_z_ wchar_t const* name,
		_In_opt_z_ wchar_t const* defaultValue = nullptr) const;
	size_t readString(
		_In_opt_z_ wchar_t const* section,
		_In_opt_z_ wchar_t const* name,
		_Out_writes_to_opt_(outSize, return +1) wchar_t* out,
		_In_ size_t outSize,
		_In_opt_z_ wchar_t const* defaultValue = nullptr) const;
	int readInt(
		_In_z_ wchar_t const* section,
		_In_z_ wchar_t const* name,
		_In_ int defaultValue = 0) const;

	void writeString(
		_In_opt_z_ wchar_t const* section,
		_In_opt_z_ wchar_t const* name,
		_In_opt_z_ wchar_t const* value) const;

	void writeInt(
		_In_opt_z_ wchar_t const* section,
		_In_opt_z_ wchar_t const* name,
		_In_ int value) const;

	//=============================
	std::wstring remoteConfig;
	std::filesystem::path plugin;
	bool	disableRepeats = 0;
	int		disableFirstKeyRepeats = 0;
	int		serverPort = 8765;
	bool	localConnectionsOnly = true;
	bool	showTrayIcon = true;
	bool	exitOnError = false;
	//=============================
	std::wstring iniFilePath;
};

/* Change this stuff */
extern std::unique_ptr<ir_remote> global_remotes;
extern std::mutex CS_global_remotes;
