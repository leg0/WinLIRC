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
 */

#include "stdafx.h"
#include "winlirc.h"
#include "drvdlg.h"
#include "resource.h"
#include "confdlg.h"
#include "remote.h"
#include "globals.h"

/////////////////////////////////////////////////////////////////////////////
// Cdrvdlg dialog


Cdrvdlg::Cdrvdlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cdrvdlg::IDD, pParent),
	  ti(IDR_TRAYMENU)
{
	//{{AFX_DATA_INIT(Cdrvdlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	use_ir_hardware=true;
	::ir_driver=&driver;
	driver.drvdlg=this;
}


void Cdrvdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cdrvdlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
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
	ON_MESSAGE(WM_TRAY, OnTrayNotification)
	//}}AFX_MSG_MAP
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
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	ti.SetNotificationWnd(this, WM_TRAY);

	if(DoInitializeDaemon()==false)
		return -1;
	
	return 0;	
}

LRESULT Cdrvdlg::OnTrayNotification(WPARAM uID, LPARAM lEvent)
{
	if(AllowTrayNotification)
		return ti.OnTrayNotification(uID, lEvent);
	else
		return 0;
}

void Cdrvdlg::OnToggleWindow() 
{
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
	if(IsWindowVisible())
		OnToggleWindow();
}

void Cdrvdlg::GoGreen()
{
	if(SetTimer(1,250,NULL))
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_RECV),"LIRC / Received Signal");
}

void Cdrvdlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent==1)
	{
		KillTimer(1);
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_OK),"LIRC / Ready");
	}
	
	CDialog::OnTimer(nIDEvent);
}

bool Cdrvdlg::DoInitializeDaemon()
{
	AllowTrayNotification=false;
	for(;;)
	{
		if(InitializeDaemon()==true)
		{
			AllowTrayNotification=true;
			return true;
		}
		
		if(!IsWindowVisible())
			OnToggleWindow();

		if(MessageBox(	"LIRC failed to initialize.\n"
						"Would you like to change the configuration\n"
						"and try again?","LIRC Error",MB_OKCANCEL)==IDCANCEL)
			return false;
		
		Cconfdlg dlg(this);
		dlg.DoModal();
	}
}

bool Cdrvdlg::InitializeDaemon()
{
	CWaitCursor foo;

	ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_INIT),"LIRC / Initializing");

	if(config.ReadConfig(&driver)==false)
	{
		DEBUG("ReadConfig failed\n");
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),"LIRC / Initialization Error");
		return false;
	}

	if(driver.InitPort(&config)==false)
	{
		DEBUG("InitPort failed\n");
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),"LIRC / Initialization Error");
		return false;
	}

	ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_OK),"LIRC / Ready");
	return true;
}

void Cdrvdlg::OnConfig() 
{
	AllowTrayNotification=false;
	Cconfdlg dlg(this);
	KillTimer(1);
	ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),"LIRC / Disabled During Configuration");
	driver.ResetPort();
	dlg.DoModal();
	if(DoInitializeDaemon()==false)
		OnCancel();
}


void Cdrvdlg::OnExitLirc() 
{
	OnCancel();
}


BOOL Cdrvdlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	GetDlgItem(IDC_VERSION)->SetWindowText(WINLIRC_VERSION);
	
	return TRUE;
}
