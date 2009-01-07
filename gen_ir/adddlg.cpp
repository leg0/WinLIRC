// This is part of the gen_ir WinLIRC Winamp Plugin DLL.
// Copyright (c) 1998 Jim Paris

// AddDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gen_ir.h"
#include "AddDlg.h"
#include "config.h"
#include "gen_ir.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddDlg dialog


CAddDlg::CAddDlg(Cconfig *nconf, CWnd* pParent /*=NULL*/)
	: CDialog(CAddDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddDlg)
	m_id = _T("");
	//}}AFX_DATA_INIT

	conf=nconf;
}


void CAddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddDlg)
	DDX_Control(pDX, IDC_ACTION, m_act);
	DDX_Text(pDX, IDC_ID, m_id);
	DDV_MaxChars(pDX, m_id, 64);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddDlg, CDialog)
	//{{AFX_MSG_MAP(CAddDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDlg message handlers

void CAddDlg::OnOK() 
{
	UpdateData();

	if(m_id.IsEmpty())
	{
		MessageBox("You need to enter a button ID.");
		return;
	}
	
	int index=m_act.GetCurSel();
	if(index==CB_ERR)
	{
		MessageBox("You need to select an action.");
		return;
	}

	for(int i=0;i<conf->our_buttons.GetSize();i++)
	{
		if(stricmp(conf->our_buttons[i],m_id)==0)
		{
			MessageBox("That button ID is already used.");
			return;
		}
	}
	
	conf->our_buttons.Add(m_id);
	conf->our_actions.Add(m_act.GetItemData(index));
	
	CDialog::OnOK();
}

BOOL CAddDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	int index;

	for(int i=0;i<TOTAL_ACTIONS;i++)
	{
		index=m_act.AddString(acts[i].name);
		if(index>=0)
			m_act.SetItemData(index,i);
	}

	GotoDlgCtrl(GetDlgItem(IDC_ID));
	
	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
