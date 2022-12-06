/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
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
 * Copyright (C) 2010 Ian Curtis
 */

#include "stdafx.h"
#include "SerialDialog.h"
#include "Globals.h"
#include "Transmit.h"


// SerialDialog dialog

IMPLEMENT_DYNAMIC(SerialDialog, CDialog)

SerialDialog::SerialDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(SerialDialog::IDD, pParent)
{
	animax			= FALSE;
	deviceType		= -1;
	virtualPulse	= 0;
	transmitterPin	= -1;
	hardwareCarrier	= FALSE;
	inverted		= FALSE;
}

void SerialDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PORT, port);
	DDX_Control(pDX, IDC_SPEED, speed);
	DDX_Control(pDX, IDC_SENSE, sense);
	DDX_Radio(pDX, IDC_RADIORX, deviceType);
	DDX_Radio(pDX, IDC_RADIODTR, transmitterPin);
	DDX_Check(pDX, IDC_CHECKANIMAX, animax);
	DDX_Check(pDX, IDC_CHECKHARDCARRIER, hardwareCarrier);
	DDX_Check(pDX, IDC_INVERTED, inverted);
	DDX_Text(pDX, IDC_VIRTPULSE, virtualPulse);
	DDV_MinMaxInt(pDX, virtualPulse, 0, 16777215);
}


BEGIN_MESSAGE_MAP(SerialDialog, CDialog)
	ON_BN_CLICKED(IDC_RADIORX, &SerialDialog::OnBnClickedRadiorx)
	ON_BN_CLICKED(IDC_RADIODCD, &SerialDialog::OnBnClickedRadiodcd)
	ON_BN_CLICKED(IDOK, &SerialDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// SerialDialog message handlers

BOOL SerialDialog::OnInitDialog() {

	//===========
	CComboBox *p;
	int x;
	//===========

	CDialog::OnInitDialog();

	settings.loadSettings();

	p = (CComboBox *)GetDlgItem(IDC_PORT);
	x = p->FindStringExact(0,settings.port);

	if(x != CB_ERR) {
		p->SetCurSel(x);
	}
	else {
		p->SetCurSel(0);
	}

	p = (CComboBox *)GetDlgItem(IDC_SPEED);
	x = p->FindStringExact(0,settings.speed);

	if(x != CB_ERR) {
		p->SetCurSel(x);
	}
	else {
		p->SetCurSel(0);
	}

	((CComboBox *)GetDlgItem(IDC_SENSE))->SetCurSel(settings.sense+1);

	animax			= settings.animax;
	hardwareCarrier	= settings.transmitterType & HARDCARRIER;
	transmitterPin	= (settings.transmitterType & TXTRANSMITTER)>>1;
	inverted		= (settings.transmitterType & INVERTED)>>2;
	virtualPulse	= settings.virtualPulse;
	deviceType		= settings.deviceType;

	UpdateData(FALSE);

	OnBnClickedRadiorx();

	return TRUE;
}

void SerialDialog::OnBnClickedRadiorx()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_SENSE)->EnableWindow(deviceType);
	GetDlgItem(IDC_VIRTPULSE)->EnableWindow(!deviceType);
	GetDlgItem(IDC_CHECKANIMAX)->EnableWindow(deviceType);
}

void SerialDialog::OnBnClickedRadiodcd()
{
	OnBnClickedRadiorx();
}

void SerialDialog::OnBnClickedOk()
{
	//===========
	CString temp;
	//===========

	OnOK();

	settings.transmitterType = (inverted<<2)|(transmitterPin<<1)|hardwareCarrier;

	int sense=((CComboBox *)GetDlgItem(IDC_SENSE))->GetCurSel();
	if(sense>=1 && sense<=2) sense--;
	else sense=-1;
	settings.sense=sense;

	port.GetWindowText(temp);
	settings.port			= temp;
	speed.GetWindowText(temp);
	settings.speed			= temp;	
	settings.deviceType		= deviceType;					
	settings.virtualPulse	= virtualPulse;
	settings.animax			= animax;

	settings.saveSettings();
}
