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

#include "winlirc.h"
#include "confdlg.h"
#include "drvdlg.h"
#include "irdriver.h"
#include "learndlg.h"
#include "config.h"
#include "remote.h"
#include "globals.h"


/////////////////////////////////////////////////////////////////////////////
// Cconfdlg dialog


Cconfdlg::Cconfdlg(Cdrvdlg *nparent, CWnd* pParent /*=NULL*/)
	: CDialog(Cconfdlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(Cconfdlg)
	m_port = _T("");
	m_filename = _T("");
	m_animax = FALSE;
	//}}AFX_DATA_INIT
	parent=nparent;
}


void Cconfdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cconfdlg)
	DDX_CBString(pDX, IDC_PORT, m_port);
	DDV_MaxChars(pDX, m_port, 64);
	DDX_Text(pDX, IDC_FILE, m_filename);
	DDV_MaxChars(pDX, m_filename, 250);
	DDX_Check(pDX, IDC_CHECK1, m_animax);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Cconfdlg, CDialog)
	//{{AFX_MSG_MAP(Cconfdlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnBrowse)
	ON_BN_CLICKED(IDC_LEARN, OnLearn)
	ON_BN_CLICKED(IDC_ANALYZE, OnAnalyze)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_RAW, OnRaw)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cconfdlg message handlers

void Cconfdlg::OnOK() 
{
	UpdateData();

	FILE *tmp;
	if(m_filename=="" || (tmp=fopen(m_filename,"r"))==NULL)
	{
		MessageBox(	"The configuration filename is invalid.\n"
					"Please try again.","Configuration Error");
		return;
	}
	fclose(tmp);

	parent->config.port=m_port;
	parent->config.animax=m_animax;
	parent->config.conf=m_filename;
	int sense=((CComboBox *)GetDlgItem(IDC_SENSE))->GetCurSel();
	if(sense>=1 && sense<=2) sense--;
	else sense=-1;
	parent->config.sense=sense;
	
	if(parent->config.WriteConfig())
		CDialog::OnOK();
	else
		CDialog::OnCancel();
}

BOOL Cconfdlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CComboBox *p=(CComboBox *)GetDlgItem(IDC_PORT);
	int x;
	if((x=p->FindStringExact(0,parent->config.port))!=CB_ERR)
	{
		p->SetCurSel(x);
		UpdateData();
	}
	else
	{
		m_port=parent->config.port;
	}
	
	m_animax=parent->config.animax;
	m_filename=parent->config.conf;
	UpdateData(false);

	((CComboBox *)GetDlgItem(IDC_SENSE))->SetCurSel(parent->config.sense+1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void Cconfdlg::OnBrowse() 
{
	CFileDialog dlg(TRUE,NULL,NULL,
		OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_NOTESTFILECREATE,
		"Configuration Files (*.cfg;*.conf;*.cf;*.rc)|*.cfg;*.conf;*.cf;*.rc|All Files (*.*)|*.*||");

	if(dlg.DoModal()!=IDOK) return;

	UpdateData();
	m_filename=dlg.GetPathName();
	UpdateData(false);
}

void Cconfdlg::OnLearn() 
{
	UpdateData();

	CWaitCursor foo;

	if(m_filename=="")
	{
		MessageBox(	"The configuration filename is invalid.\n"
					"Please enter the name of a non-existent\n"
					"file and try again.","Error");
		return;
	}
	FILE *tmp;
	if((tmp=fopen(m_filename,"r"))!=NULL)
	{
		fclose(tmp);
		if(MessageBox( "The configuration file already exists.\n"
					"Do you wish to overwrite it?","LIRC",MB_YESNO)==IDNO)
			return;
	}
	
	if((tmp=fopen(m_filename,"w"))==NULL)
	{
		MessageBox( "Cannot create the configuration file.\n"
					"Please try again.","Error");
		return;
	}
	fclose(tmp);

	parent->config.port=m_port;
	parent->config.animax=m_animax;
	parent->config.conf=m_filename;
	int sense=((CComboBox *)GetDlgItem(IDC_SENSE))->GetCurSel();
	if(sense>=1 && sense<=2) sense--;
	else sense=-1;
	parent->config.sense=sense;


	parent->ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_INIT),"LIRC / Initializing");

	if(parent->driver.InitPort(&parent->config,false)==false)
	{
		parent->ti.SetIcon(
			AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),
			"LIRC / Initialization Error");
		MessageBox(	"There was an error initializing LIRC.\n"
					"Please check the port settings and try again.\n","Error");
		fclose(tmp);
		return;
	}

	parent->ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_OK),"LIRC / Ready");

	Clearndlg dlg(&parent->driver,m_filename,lm_learn);
	int result=dlg.DoModal();
	parent->ti.SetIcon(
		AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),
		"LIRC / Disabled During Configuration");
	parent->driver.ResetPort();
	if(result==IDOK)
		MessageBox("Don't forget to 'analyze' this data for better performance.",
			"Remote successfully configured");
}


void Cconfdlg::OnAnalyze() 
{
	UpdateData();

	CWaitCursor foo;

	if(m_filename=="")
	{
		MessageBox(	"The configuration filename is invalid.\n"
					"Please enter the name of an existing\n"
					"file and try again.","Error");
		return;
	}
	FILE *tmp;
	if((tmp=fopen(m_filename,"r"))==NULL)
	{
		MessageBox("Error opening configuration file.");
		return;
	}
	fclose(tmp);

	Clearndlg dlg(&parent->driver,m_filename,lm_analyze);
	DEBUG("Analyze DoModal() call\n");
	dlg.DoModal();
	DEBUG("Analyze DoModal() returned\n");
}

void Cconfdlg::OnRaw() 
{
	UpdateData();

	CWaitCursor foo;

	if(m_filename=="")
	{
		MessageBox(	"The configuration filename is invalid.\n"
					"Please enter the name of an existing\n"
					"file and try again.","Error");
		return;
	}

	FILE *tmp;
	if((tmp=fopen(m_filename,"r"))==NULL)
	{
		MessageBox("Error opening configuration file.");
		return;
	}
	fclose(tmp);

	parent->config.port=m_port;
	parent->config.animax=m_animax;
	parent->config.conf=m_filename;
	int sense=((CComboBox *)GetDlgItem(IDC_SENSE))->GetCurSel();
	if(sense>=1 && sense<=2) sense--;
	else sense=-1;
	parent->config.sense=sense;

	parent->ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_INIT),"LIRC / Initializing");

	if(parent->driver.InitPort(&parent->config,false)==false)
	{
		parent->ti.SetIcon(
			AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),
			"LIRC / Initialization Error");
		MessageBox(	"There was an error initializing LIRC.\n"
					"Please check the port settings and try again.\n","Error");
		return;
	}

	parent->ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_OK),"LIRC / Ready");

	Clearndlg dlg(&parent->driver,m_filename,lm_raw);
	int result=dlg.DoModal();
	parent->ti.SetIcon(
		AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),
		"LIRC / Disabled During Configuration");
	parent->driver.ResetPort();
}
