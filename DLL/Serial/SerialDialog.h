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

#pragma once
#include "afxwin.h"
#include "resource.h"


// SerialDialog dialog

class SerialDialog : public CDialog
{
	DECLARE_DYNAMIC(SerialDialog)

public:
	SerialDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~SerialDialog();

// Dialog Data
	enum { IDD = IDD_SERIALDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	
private:
	CComboBox port;
	CComboBox speed;
	CComboBox sense;
	BOOL animax;
	BOOL inverted;
	BOOL hardwareCarrier;
	int virtualPulse;
	int transmitterPin;
	int	deviceType;
public:
	afx_msg void OnBnClickedRadiorx();
	afx_msg void OnBnClickedRadiodcd();
	afx_msg void OnBnClickedOk();
	DECLARE_MESSAGE_MAP()
};
