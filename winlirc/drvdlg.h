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
 * Modifications Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 */

#pragma once

#include "trayicon.h"
#include "irdriver.h"
#include "irconfig.h"
#include "resource.h"

class Cdrvdlg : public CDialog
{
public:
	Cdrvdlg(CWnd* pParent = nullptr);
	
	bool initialized;
	bool AllowTrayNotification;
	bool DoInitializeDaemon();
	bool InitializeDaemon();
	void GoGreen();
	void GoBlue();	//turns the tray icon blue to indicate a transmission
	
	// Dialog Data
	enum { IDD = IDD_DIALOG };
	CComboBox	m_IrCodeEditCombo;
	CComboBox	m_remote_DropDown;
	CString	m_ircode_edit;
	CString	m_remote_edit;
	int	m_reps_edit;

	CTrayIcon ti;
	CIRDriver driver;

protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	virtual void OnOK() override;
	virtual void OnCancel() override;
	virtual BOOL OnInitDialog() override;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnToggleWindow();
	afx_msg void OnConfig();
	afx_msg void OnHideme();
	afx_msg void OnExitLirc();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSendcode();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnDropdownIrcodeEdit();
	afx_msg LRESULT OnPowerBroadcast(WPARAM uPowerEvent, LPARAM lP);
	void UpdateRemoteComboLists();
	void UpdateIrCodeComboLists();
	LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);

	DECLARE_MESSAGE_MAP()
};
