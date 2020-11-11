#pragma once

#include <Windows.h>
#include <atlstr.h>
#include "../../winlirc/Plugin.h"
#include <winlirc/WLPluginAPI.h>
#include <filesystem>

class CIRDriver
{
public:
	bool	loadPlugin	(std::filesystem::path plugin);
	void	unloadPlugin();
	bool	init		() const;
	void	deinit		() const;
	int		sendIR		(ir_remote *remote, ir_ncode *code, int repeats) const;
	int		decodeIR	(ir_remote *remote, char *out, size_t out_size) const;

	hardware const* getHardware() const;

private:
	Plugin plugin_;
};
