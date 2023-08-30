#include "stdafx.h"
#include "irdriver.h"
#include "irconfig.h"
#include "config.h"
#include "drvdlg.h"
#include "server.h"
#include "winlircapp.h"

#include <spdlog/spdlog.h>

CIRDriver::CIRDriver() noexcept
	: winlirc_api{
		.plugin_api_version = winlirc_plugin_api_version,
		.getExitEvent = [](winlirc_api const* wa) {
			auto const self = static_cast<CIRDriver const*>(wa);
			return reinterpret_cast<WLEventHandle>(self->m_pluginStopEvent.get());
		}
	}
{ }

CIRDriver::~CIRDriver()
{
	unloadPlugin();
	ASSERT(!m_daemonThreadHandle.joinable());
}

bool CIRDriver::loadPlugin(std::filesystem::path plugin)
{
	std::lock_guard lock{ m_dllLock };
	unloadPlugin(); // make sure we have cleaned up first

	Plugin p{ plugin };
	if (p.hasValidInterface())
	{
		m_dll = std::move(p);
		return true;
	}

	return false;
}

void CIRDriver::unloadPlugin()
{
	deinit(); // make sure we have cleaned up

	// daemon thread should not be dead now.
	ASSERT(!m_daemonThreadHandle.joinable());
	m_dll = Plugin{ };
}

bool CIRDriver::init()
{
	deinit();

	// daemon thread should be dead now.
	ASSERT(!m_daemonThreadHandle.joinable());

	if (auto pluginInit = m_dll.interface_->init)
	{
		m_pluginStopEvent = winlirc::Event::manualResetEvent();

		if (pluginInit(m_dll.interface_, this))
		{
			m_daemonThreadHandle = std::jthread{ [&](std::stop_token stop) {
				::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_IDLE);
				this->DaemonThreadProc(stop);
			} };
			return true;
		}
		else
		{
			deinit();
		}
	}

	return false;
}

void CIRDriver::deinit()
{
	stopDaemonThread();

	if(auto pluginDeinit = m_dll.interface_->deinit)
		pluginDeinit(m_dll.interface_);
}

int CIRDriver::sendIR(ir_remote* remote, ir_ncode* code, int repeats) const
{
	std::lock_guard lock{ m_dllLock };
	if (auto pluginSendIr = m_dll.interface_->sendIR)
		return pluginSendIr(m_dll.interface_, remote, code, repeats);
	return 0;
}

int CIRDriver::decodeIR(ir_remote* remote, char* out, size_t out_size) const
{
	std::lock_guard lock{ m_dllLock };
	if(auto pluginDecodeIr = m_dll.interface_->decodeIR)
		return pluginDecodeIr(m_dll.interface_, remote,out, out_size);
	return 0;
}

int CIRDriver::setTransmitters(unsigned int transmitterMask) const
{
	std::lock_guard lock{ m_dllLock };
	if(auto pluginSetTransmitters = m_dll.interface_->setTransmitters)
		return pluginSetTransmitters(m_dll.interface_, transmitterMask);
	return 0;
}

void CIRDriver::DaemonThreadProc(std::stop_token stop) const {

	/* Accept client connections,		*/
	/* and watch the data buffer.		*/
	/* When data comes in, decode it	*/
	/* and send the result to clients.	*/

	char message[PACKET_SIZE+1];
	auto gr = app.config->use_global_remotes([&](ir_remote* global_remotes) {
		return std::make_unique<ir_remote>(*global_remotes);
	});
	auto decodeIr = [&]() {
		std::lock_guard lock{ m_dllLock };
		auto pluginDecodeIr = m_dll.interface_->decodeIR;
		ASSERT(pluginDecodeIr != nullptr);
		auto const res = pluginDecodeIr(m_dll.interface_, &*gr, message, sizeof(message));
		if (res)
			spdlog::debug("decodeIR: res={}, {}", res, message);
		else
			spdlog::debug("decodeIR: res={}", res);
		return res;
	};

	while(!stop.stop_requested()) {

		if(decodeIr()) {

			//======================
			UINT64	keyCode;
			INT		repeat;
			CHAR	command[128];
			CHAR	remoteName[128];
			//======================

			auto& config = *app.config;
			if(config.disableRepeats) {

				if(sscanf(message,"%I64x %x",&keyCode,&repeat)==2) {
				
					if(repeat) continue;
				}
			}
			else if(config.disableFirstKeyRepeats>0) {
				
				if(sscanf(message,"%I64x %x %s %s",&keyCode,&repeat,command,remoteName)==4) {
				
					if(repeat) {

						if(repeat<=config.disableFirstKeyRepeats) continue;
						else {
							sprintf(message,"%016llx %02x %s %s\n",keyCode,repeat-config.disableFirstKeyRepeats,command,remoteName);
						}
					}
				}
			}

			app.dlg->GoGreen();
			app.server.sendToClients(message);
		}
	}
}

void CIRDriver::stopDaemonThread()
{
	if (m_daemonThreadHandle.joinable())
	{
		m_pluginStopEvent.setEvent();
		m_daemonThreadHandle.join();
	}

	// daemon thread should be dead now.
	ASSERT(!m_daemonThreadHandle.joinable());
}
