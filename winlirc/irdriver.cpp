#include "stdafx.h"
#include "irdriver.h"
#include "irconfig.h"
#include "IRRemote.h"
#include "Receive.h"
#include "Send.h"
#include "config.h"
#include "drvdlg.h"
#include "server.h"
#include "winlirc.h"

CIRDriver::CIRDriver()
	: m_daemonThreadEvent{ CreateEvent(nullptr, TRUE, FALSE, nullptr) }
{ }

CIRDriver::~CIRDriver()
{
	unloadPlugin();
	ASSERT(!m_daemonThreadHandle.joinable());
	if(m_daemonThreadEvent)
		CloseHandle(m_daemonThreadEvent);
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

static winlirc_interface const Winlirc{
	.map_code = &winlirc_map_code,
	.map_gap = &winlirc_map_gap,
	.decodeCommand = &winlirc_decodeCommand,
	.get_ir_code = &get_ir_code,
	.get_next_ir_code_node = &get_next_ir_code_node,
	.bit_count = &bit_count,
	.has_repeat = &has_repeat,
	.set_protocol = &set_protocol,
	.is_raw = &is_raw,
	.is_space_enc = &is_space_enc,
	.is_space_first = &is_space_first,
	.is_rc5 = &is_rc5,
	.is_rc6 = &is_rc6,
	.is_biphase = &is_biphase,
	.is_rcmm = &is_rcmm,
	.is_goldstar = &is_goldstar,
	.is_grundig = &is_grundig,
	.is_bo = &is_bo,
	.is_serial = &is_serial,
	.is_xmp = &is_xmp,
	.is_const = &is_const,
	.has_repeat_gap = &has_repeat_gap,
	.has_pre = &has_pre,
	.has_post = &has_post,
	.has_header = &has_header,
	.has_foot = &has_foot,
	.has_toggle_bit_mask = &has_toggle_bit_mask,
	.has_ignore_mask = &has_ignore_mask,
	.has_toggle_mask = &has_toggle_mask,
	.min_gap = &min_gap,
	.max_gap = &max_gap,
	.gen_ir_code = &gen_ir_code,
	.match_ir_code = &match_ir_code,
	.expect = &expect,
	.expect_at_least = &expect_at_least,
	.expect_at_most = &expect_at_most,
	.get_freq = &get_freq,
	.set_freq = &set_freq,
	.get_duty_cycle = &get_duty_cycle,
	.set_duty_cycle = &set_duty_cycle,
	.init_rec_buffer = &winlirc_init_rec_buffer,
	.clear_rec_buffer = &winlirc_clear_rec_buffer,
	.receive_decode = &winlirc_receive_decode,
	.get_send_buffer_length = &winlirc_get_send_buffer_length,
	.get_send_buffer_data = &winlirc_get_send_buffer_data,
	.init_send_buffer = &winlirc_init_send_buffer,
	.init_send = &winlirc_init_send
};

bool CIRDriver::init()
{
	deinit();

	// daemon thread should be dead now.
	ASSERT(!m_daemonThreadHandle.joinable());

	if (auto pluginInit = m_dll.interface_.init)
	{
		if (pluginInit(&Winlirc))
		{
			m_daemonThreadHandle = std::thread{ [&]() {
				::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_IDLE);
				this->DaemonThreadProc();
			} };

			if (m_daemonThreadHandle.joinable())
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

	if(auto pluginDeinit = m_dll.interface_.deinit)
		pluginDeinit();
}

int CIRDriver::sendIR(ir_remote* remote, ir_ncode* code, int repeats) const
{
	std::lock_guard lock{ m_dllLock };
	if (auto pluginSendIr = m_dll.interface_.sendIR)
		return pluginSendIr(remote, code, repeats);
	return 0;
}

int CIRDriver::decodeIR(ir_remote* remote, char* out, size_t out_size) const
{
	std::lock_guard lock{ m_dllLock };
	if(auto pluginDecodeIr = m_dll.interface_.decodeIR)
		return pluginDecodeIr(remote,out, out_size);
	return 0;
}

int CIRDriver::setTransmitters(unsigned int transmitterMask) const
{
	std::lock_guard lock{ m_dllLock };
	if(auto pluginSetTransmitters = m_dll.interface_.setTransmitters)
		return pluginSetTransmitters(transmitterMask);
	return 0;
}

void CIRDriver::DaemonThreadProc() const {

	/* Accept client connections,		*/
	/* and watch the data buffer.		*/
	/* When data comes in, decode it	*/
	/* and send the result to clients.	*/

	char message[PACKET_SIZE+1];
	auto decodeIr = [&]() {
		std::lock_guard lock{ m_dllLock };
		std::lock_guard lock2{ CS_global_remotes };

		auto pluginDecodeIr = m_dll.interface_.decodeIR;
		ASSERT(pluginDecodeIr != nullptr);
		return pluginDecodeIr(global_remotes.get(), message, sizeof(message));
	};

	while(WaitForSingleObject(m_daemonThreadEvent, 0) == WAIT_TIMEOUT) {

		if(decodeIr()) {

			//======================
			UINT64	keyCode;
			INT		repeat;
			CHAR	command[128];
			CHAR	remoteName[128];
			//======================

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
		::SetEvent(m_daemonThreadEvent);
		m_daemonThreadHandle.join();
	}

	// daemon thread should be dead now.
	ASSERT(!m_daemonThreadHandle.joinable());
}
