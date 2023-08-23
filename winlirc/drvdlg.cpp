/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * Modifications based on LIRC 0.6.x Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#include "stdafx.h"
#include "winlircapp.h"
#include "drvdlg.h"
#include "resource.h"
#include "remote.h"
#include "server.h" //so we can send SIGHUP
#include "InputPlugin.h"

#include <spdlog/spdlog.h>

#include <source_location>
#include <string>

template<>
struct fmt::formatter<std::source_location> : fmt::formatter<std::string>
{
	auto format(std::source_location sl, format_context& ctx) const
	{
		return std::format_to(ctx.out(), "{}({})", sl.file_name(), sl.line());
	}
};

using namespace std::string_literals;

#define WM_TRAY (WM_USER+34)

/////////////////////////////////////////////////////////////////////////////
// Cdrvdlg dialog


Cdrvdlg::Cdrvdlg(CWnd* pParent)
	: CDialog(Cdrvdlg::IDD, pParent),
	  ti(IDR_TRAYMENU)
{
	//{{AFX_DATA_INIT(Cdrvdlg)
	m_ircode_edit	= _T("");
	m_remote_edit	= _T("");
	m_reps_edit		= 0;
	//}}AFX_DATA_INIT
}


void Cdrvdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cdrvdlg)
	DDX_Control(pDX, IDC_IRCODE_EDIT, m_IrCodeEditCombo);
	DDX_Control(pDX, IDC_REMOTE_EDIT, m_remote_DropDown);
	DDX_Text(pDX, IDC_IRCODE_EDIT, m_ircode_edit);
	DDV_MaxChars(pDX, m_ircode_edit, 64);
	DDX_Text(pDX, IDC_REMOTE_EDIT, m_remote_edit);
	DDV_MaxChars(pDX, m_remote_edit, 64);
	DDX_Text(pDX, IDC_REPS_EDIT, m_reps_edit);
	DDV_MinMaxInt(pDX, m_reps_edit, 0, 600);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Cdrvdlg, CDialog)
	//{{AFX_MSG_MAP(Cdrvdlg)
	ON_WM_CREATE()
	ON_COMMAND(ID_TOGGLEWINDOW, OnToggleWindow)
	ON_BN_CLICKED(IDC_CONFIG, OnConfig)
	ON_BN_CLICKED(IDC_HIDEME, OnHideme)
	ON_COMMAND(ID_EXITLIRC, OnExitLirc)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SENDCODE, OnSendcode)
	ON_WM_COPYDATA()
	ON_CBN_DROPDOWN(IDC_IRCODE_EDIT, OnDropdownIrcodeEdit)
	ON_MESSAGE(WM_TRAY, OnTrayNotification)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cdrvdlg message handlers

void Cdrvdlg::OnOK() 
{

	CDialog::OnOK();
	ti.SetIcon(0);
	DestroyWindow();
}

void Cdrvdlg::OnCancel() 
{
	CDialog::OnCancel();
	ti.SetIcon(0);
	DestroyWindow();
}

int Cdrvdlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	spdlog::debug("{}", std::source_location::current());

	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	ti.SetNotificationWnd(this, WM_TRAY);

	if(DoInitializeDaemon()==false)
		return -1;
	
	return 0;	
}

afx_msg LRESULT Cdrvdlg::OnPowerBroadcast(WPARAM wPowerEvent,LPARAM lP)
{
	spdlog::debug("{}", std::source_location::current());

	LRESULT retval = TRUE;

	switch (wPowerEvent)
	{
		case PBT_APMQUERYSUSPEND:
			{
				//can we suspend?
				//bit 0 false => automated or emergency power down 
				//UI iff bit 0 is set
				BOOL bUIEnabled=(lP & 0x00000001)!=0;
				retval = TRUE;
				break;
			}
		case PBT_APMSUSPEND:
			{
				driver.deinit(); //if we RequestDeviceWakeup instead we could wake on irevents
				retval = TRUE;		//if the RI pin was connected to the receiving device
				break;
			}

			//wake up from a critical power shutdown
		case PBT_APMRESUMECRITICAL:
			//standard wake up evrnt; UI is on
			break;
		case PBT_APMRESUMESUSPEND:
			//New for Win98;NT5
			//unattended wake up; no user interaction possible; screen
			//is probably off.
			break;
		case PBT_APMRESUMEAUTOMATIC:
		{
			//system power source has changed, or 
			//battery life has just got into the near-critical state
			auto& config = *app.config;

			if(config.showTrayIcon) {
				ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_INIT),_T("WinLIRC / Initializing"));
			}

			Sleep(1000);

			if(driver.init()==false) {
				spdlog::debug("InitPort failed");
				if(config.showTrayIcon) ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),_T("WinLIRC / Initialization Error"));
				retval = false;
				break;
			}

			if(config.showTrayIcon) {
				ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_OK),_T("WinLIRC / Ready"));
			}

			retval = TRUE;
			break;
		}
		case PBT_APMPOWERSTATUSCHANGE:
			retval = TRUE;
			break;
		default:
			retval=TRUE;
			break;
		}

		return retval;
	}

LRESULT Cdrvdlg::OnTrayNotification(WPARAM uID, LPARAM lEvent)
{
	spdlog::debug("{}", std::source_location::current());

	if(AllowTrayNotification)
		return ti.OnTrayNotification(uID, lEvent);
	else
		return 0;
}

void Cdrvdlg::OnToggleWindow() 
{
	spdlog::debug("{}", std::source_location::current());

	if(IsWindowVisible())
	{
		ShowWindow(SW_HIDE);
		UpdateWindow();
	}
	else
	{
		ShowWindow(SW_SHOW);
		SetForegroundWindow();
		UpdateWindow();
	}
}

void Cdrvdlg::OnHideme() 
{
	spdlog::debug("{}", std::source_location::current());

	if(IsWindowVisible())
		OnToggleWindow();
}

void Cdrvdlg::GoGreen()
{
	spdlog::debug("{}", std::source_location::current());

	if(!app.config->showTrayIcon) {
		return;
	}

	if(SetTimer(1,250,nullptr)) {
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_RECV),_T("WinLIRC / Received Signal"));
	}
}
void Cdrvdlg::GoBlue()
{
	spdlog::debug("{}", std::source_location::current());

	if(!app.config->showTrayIcon) {
		return;
	}

	if(SetTimer(1,250,nullptr)) {
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_SEND),_T("WinLIRC / Sent Signal"));
	}
}

void Cdrvdlg::OnTimer(UINT_PTR nIDEvent) {

	spdlog::debug("{}", std::source_location::current());

	if(nIDEvent==1) {
		KillTimer(1);
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_OK),_T("WinLIRC / Ready"));
	}
	
	CDialog::OnTimer(nIDEvent);
}

bool Cdrvdlg::DoInitializeDaemon()
{
	spdlog::debug("{}", std::source_location::current());

	AllowTrayNotification=false;
	for(;;)
	{
		if(InitializeDaemon())
		{
			AllowTrayNotification=true;
			return true;
		}
		else {
			//printf("failed here :(\n");
		}

		if(app.config->exitOnError) {
			ti.DisableTrayIcon();
			exit(0);
		}
		
		if(!IsWindowVisible())
			OnToggleWindow();

		if(MessageBox(	_T("WinLIRC failed to initialize.\n")
						_T("Would you like to change the configuration\n")
						_T("and try again?"),_T("WinLIRC Error"),MB_OKCANCEL)==IDCANCEL)
			return false;
		
		InputPlugin inputPlugin(this);
		inputPlugin.DoModal();
	}
}

bool Cdrvdlg::InitializeDaemon() {

	spdlog::debug("{}", std::source_location::current());

	//==============
	CWaitCursor foo;
	//==============

	auto& config = *app.config;

	if (!config.remoteConfig.empty())
	{
		if(!config.readConfig()) {

			if(!config.exitOnError) MessageBox(	_T("Error loading config file."),_T("Configuration Error"));
			if(config.showTrayIcon) ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),_T("WinLIRC / Initialization Error"));
			return false;
		}
	}

	if(!config.showTrayIcon) {
		ti.DisableTrayIcon();
	}
	
	if(!driver.loadPlugin(config.plugin)) {
		if(config.showTrayIcon) ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),_T("WinLIRC / Initialization Error"));
		return false;
	}

	if(config.showTrayIcon) {
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_INIT),_T("WinLIRC / Initializing"));
	}

	if(!driver.init()) {
		if(config.showTrayIcon) ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),_T("WinLIRC / Initialization Error"));
		return false;
	}
	
	app.server.sendToClients("BEGIN\nSIGHUP\nEND\n");
	if(config.showTrayIcon) ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_OK),_T("WinLIRC / Ready"));
	return true;
}

void Cdrvdlg::OnConfig() 
{
	driver.deinit();

	InputPlugin inputPlugin(this);
	inputPlugin.DoModal();

	AllowTrayNotification=false;
	KillTimer(1);
	if(app.config->showTrayIcon) ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),_T("WinLIRC / Disabled During Configuration"));

	if(!DoInitializeDaemon())
		OnCancel();

	UpdateRemoteComboLists();
}


void Cdrvdlg::OnExitLirc() 
{
	spdlog::debug("{}", std::source_location::current());

	OnCancel();
}


BOOL Cdrvdlg::OnInitDialog() 
{
	spdlog::debug("{}", std::source_location::current());

	CDialog::OnInitDialog();
	m_ircode_edit="";
	UpdateData(FALSE);
	UpdateRemoteComboLists();
	return TRUE;
}

void Cdrvdlg::OnSendcode()
{
	spdlog::debug("{}", std::source_location::current());

	EnableWindow(FALSE);
	UpdateData(TRUE);

	m_remote_edit.TrimRight();
	m_ircode_edit.TrimRight();
	m_remote_edit.TrimLeft();
	m_ircode_edit.TrimLeft();

	app.config->use_global_remotes([&](ir_remote* global_remotes) {

		USES_CONVERSION;
		const char* remoteName = T2A(m_remote_edit);
		const char* codeName = T2A(m_ircode_edit);

		ir_remote* sender = get_remote_by_name(global_remotes, remoteName);

		if (sender == nullptr) {
			MessageBox(_T("No match found for remote!"));
		}
		else {

			auto codes = get_code_by_name(sender->codes, codeName);

			if (codes == end(sender->codes)) {
				MessageBox(_T("No match found for ircode!"));
			}
			else {

				if (m_reps_edit < sender->min_repeat) {
					m_reps_edit = sender->min_repeat;		//set minimum number of repeats
					UpdateData(FALSE);						//update the display
				}

				//reset toggle masks

				if (has_toggle_mask(sender)) {
					sender->toggle_mask_state = 0;
				}

				if (has_toggle_bit_mask(sender)) {
					sender->toggle_bit_mask_state = (sender->toggle_bit_mask_state ^ sender->toggle_bit_mask);
				}

				//send code

				if (driver.sendIR(sender, &*codes, m_reps_edit)) {
					GoBlue();
				}
				else {
					MessageBox(_T("Send failed/not supported."));
				}
			}
		}
	});

	UpdateData(FALSE);
	EnableWindow(TRUE);
}

BOOL Cdrvdlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
// handles a transmission command recieved from another application
// pCopyDataStruct->lpData should point to a string of the following format
// remotename	ircodename	reps
// where reps is an optional parameter indicating the number of times to repeat the code (default=0)
{
	auto cmd = "SEND_ONCE "s + static_cast<char const*>(pCopyDataStruct->lpData);
	return app.server.parseSendString(cmd.c_str()).first;
}

void Cdrvdlg::UpdateRemoteComboLists()
{
	spdlog::debug("{}", std::source_location::current());

	USES_CONVERSION;
	UpdateData(TRUE);
	m_remote_DropDown.ResetContent();

	app.config->use_global_remotes([&](ir_remote const* global_remotes) {
		//Fill remote combo box
		ir_remote const* sender = global_remotes;
		while (sender != nullptr)
		{
			m_remote_DropDown.AddString(A2T(sender->name.c_str()));
			sender = sender->next.get();
		}
	});

	//Set selected item
	if (m_remote_DropDown.SelectString(-1,m_remote_edit) == CB_ERR)
	{
		//Did not find remote selected before, select first
		m_remote_DropDown.SetCurSel(0);
	}
	UpdateData(FALSE);

	UpdateIrCodeComboLists();
}

void Cdrvdlg::UpdateIrCodeComboLists()
{
	spdlog::debug("{}", std::source_location::current());

	USES_CONVERSION;
	UpdateData(TRUE);
	
	//Retrieve pointer to remote by name
	app.config->use_global_remotes([&](ir_remote* global_remotes) {
		ir_remote* selected_remote = get_remote_by_name(global_remotes, T2A(m_remote_edit.GetBuffer()));

		m_IrCodeEditCombo.ResetContent();

		if (selected_remote)
		{
			for (auto& c : selected_remote->codes)
			{
				m_IrCodeEditCombo.AddString(A2T(c.name->c_str()));
			}
		}
	});

	if (m_IrCodeEditCombo.SelectString(-1,m_ircode_edit) == CB_ERR)
	{
		m_IrCodeEditCombo.SetCurSel(0);
	}

	UpdateData(FALSE);
}

void Cdrvdlg::OnDropdownIrcodeEdit() 
{
	spdlog::debug("{}", std::source_location::current());

	// TODO: Add your control notification handler code here
	UpdateIrCodeComboLists();
}
