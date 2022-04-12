#include <Windows.h>
#include "irdriver.h"

bool CIRDriver::loadPlugin(std::filesystem::path plugin)
{
	unloadPlugin();

	Plugin p{ plugin };
	if (p.hasValidInterface())
	{
		plugin_ = std::move(p);
		return true;
	}
	return false;
}

void CIRDriver::unloadPlugin()
{
	plugin_ = Plugin{};
}

bool CIRDriver::init() const
{
	deinit();
	if (auto initFn = plugin_.interface_.init)
		return initFn(0);
	else
		return false;
}

void CIRDriver::deinit() const
{
	if (auto deinitFn = plugin_.interface_.deinit)
		deinitFn();
}

int	CIRDriver::sendIR(ir_remote* remote, ir_ncode* code, int repeats) const
{
	if (auto sendFn = plugin_.interface_.sendIR)
		return sendFn(remote, code, repeats);
	else
		return 0;
}

int	CIRDriver::decodeIR(struct ir_remote *remote, size_t remotes_count, char *out, size_t out_size) const
{
	if (auto decodeFn = plugin_.interface_.decodeIR)
		return decodeFn(remote, remotes_count, out, out_size);
	else
		return 0;
}

hardware const* CIRDriver::getHardware() const
{
	if (plugin_.interface_.hardware)
		return plugin_.interface_.hardware;

	if(auto getHwFn = plugin_.interface_.getHardware)
		return getHwFn();

	return nullptr;
}
