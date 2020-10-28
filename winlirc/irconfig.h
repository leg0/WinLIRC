#pragma once

#include "globals.h"

#include "stdafx.h"
#include "irdriver.h"
#include <filesystem>
#include <mutex>

class CIRConfig
{
public:
	CIRConfig();
	~CIRConfig();

	bool readConfig	();
	bool writeINIFile();
	bool readINIFile();

	//=============================
	CString remoteConfig;
	std::filesystem::path plugin;
	BOOL	disableRepeats;
	INT		disableFirstKeyRepeats;
	INT		serverPort;
	BOOL	localConnectionsOnly;
	BOOL	showTrayIcon;
	BOOL	exitOnError;
	//=============================
};

/* Change this stuff */
extern ir_remote* global_remotes;
extern std::mutex CS_global_remotes;
extern class CIRConfig config;
