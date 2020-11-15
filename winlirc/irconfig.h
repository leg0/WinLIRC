#pragma once

#include "irdriver.h"
#include <filesystem>
#include <mutex>
#include <string>

class CIRConfig
{
public:
	CIRConfig();
	~CIRConfig();

	bool readConfig	();
	bool writeINIFile();
	bool readINIFile();

	//=============================
	std::wstring remoteConfig;
	std::filesystem::path plugin;
	bool	disableRepeats;
	int		disableFirstKeyRepeats;
	int		serverPort;
	bool	localConnectionsOnly;
	bool	showTrayIcon;
	bool	exitOnError;
	//=============================
};

/* Change this stuff */
extern ir_remote* global_remotes;
extern std::mutex CS_global_remotes;
extern class CIRConfig config;
