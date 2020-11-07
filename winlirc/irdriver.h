#pragma once

#include "Plugin.h"
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>

struct ir_remote;
struct ir_ncode;

class CIRDriver
{
public:
	CIRDriver();
	~CIRDriver();

	BOOL	loadPlugin		(std::filesystem::path plugin);
	void	unloadPlugin	();
	BOOL	init			();
	void	deinit			();
	int		sendIR			(ir_remote* remote, ir_ncode* code, int repeats);
	int		decodeIR		(ir_remote* remote, char* out, size_t out_size);
	int		setTransmitters	(unsigned int transmitterMask);

	void	DaemonThreadProc() const;

private:

	void stopDaemonThread();

	/// Protects access to the functions imported from plug-in dll, and the
	/// DLL handle.
	mutable std::mutex m_dllLock;

	//===============================
	Plugin		m_dll;
	HANDLE const m_daemonThreadEvent;
	std::thread m_daemonThreadHandle;
	//===============================
};
