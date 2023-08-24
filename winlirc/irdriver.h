#pragma once

#include "../Common/Event.h"
#include "Plugin.h"
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>

struct ir_remote;
struct ir_ncode;

class CIRDriver : private winlirc_api
{
public:
	CIRDriver() noexcept;
	~CIRDriver();

	bool loadPlugin(std::filesystem::path plugin);
	void unloadPlugin();
	bool init();
	void deinit();
	int  sendIR(ir_remote* remote, ir_ncode* code, int repeats) const;
	int  decodeIR(ir_remote* remote, char* out, size_t out_size) const;
	int  setTransmitters(unsigned int transmitterMask) const;


private:
	void DaemonThreadProc(std::stop_token stop) const;
	void stopDaemonThread();

	/// Protects access to the functions imported from plug-in dll, and the
	/// DLL handle.
	mutable std::mutex m_dllLock;
	Plugin m_dll;
	winlirc::Event m_pluginStopEvent;
	std::jthread m_daemonThreadHandle;
};
