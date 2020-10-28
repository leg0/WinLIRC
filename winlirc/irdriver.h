#pragma once

#include "Plugin.h"
#include <mutex>
#include <string>

struct ir_remote;
struct ir_ncode;

class CIRDriver
{
public:
	CIRDriver();
	~CIRDriver();

	BOOL	loadPlugin		(std::wstring plugin);
	void	unloadPlugin	();
	BOOL	init			();
	void	deinit			();
	int		sendIR			(ir_remote* remote, ir_ncode* code, int repeats);
	int		decodeIR		(ir_remote* remote, char* out, size_t out_size);
	int		setTransmitters	(unsigned int transmitterMask);

	void	DaemonThreadProc() const;

private:

	/// Protects access to the functions imported from plug-in dll, and the
	/// DLL handle.
	mutable std::mutex m_dllLock;

	//===============================
	Plugin		m_dll;
	std::wstring m_loadedPlugin;
	HANDLE m_daemonThreadEvent;
	CWinThread*	m_daemonThreadHandle;
	//===============================
};
