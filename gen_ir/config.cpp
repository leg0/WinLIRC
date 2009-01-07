// config.cpp : implementation file
//

#include "stdafx.h"
#include "gen_ir.h"
#include "config.h"
#include "adddlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Cconfig dialog


Cconfig::Cconfig(CGen_irApp *napp, CWnd* pParent /*=NULL*/)
	: CDialog(Cconfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(Cconfig)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	app=napp;
}


void Cconfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cconfig)
	DDX_Control(pDX, IDC_CONF, m_conf);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Cconfig, CDialog)
	//{{AFX_MSG_MAP(Cconfig)
	ON_NOTIFY(NM_CLICK, IDC_CONF, OnClickConf)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDAPPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cconfig message handlers

void Cconfig::OnClickConf(NMHDR* pNMHDR, LRESULT* pResult) 
{

	*pResult = 0;
}

void Cconfig::RefreshList(void)
{
	int max;
	int item;
	if((max=our_buttons.GetSize()) != our_actions.GetSize())
		return;
	m_conf.DeleteAllItems();
	for(int i=0;i<max;i++)
	{
		item=m_conf.InsertItem(0,our_buttons[i]);
		m_conf.SetItem(item,1,LVIF_TEXT,acts[our_actions[i]].name,0,0,0,0);

	}
}


BOOL Cconfig::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// make our copy
	int i;
	for(i=0;i<app->buttons.GetSize();i++)
		our_buttons.Add(app->buttons[i]);
	for(i=0;i<app->actions.GetSize();i++)
		our_actions.Add(app->actions[i]);

	m_conf.InsertColumn(0,"Button");
	m_conf.InsertColumn(1,"Action");
	RECT foo;
	m_conf.GetClientRect(&foo);
	m_conf.SetColumnWidth(0,(foo.right*95)/200);
	m_conf.SetColumnWidth(1,(foo.right*95)/200);
	
	RefreshList();
	GetDlgItem(IDAPPLY)->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void Cconfig::OnAdd() 
{
	CAddDlg foo(this);
	if(foo.DoModal()==IDOK)
	{
		RefreshList();
		GetDlgItem(IDAPPLY)->EnableWindow(TRUE);
	}
}

void Cconfig::OnDelete() 
{
	int index=m_conf.GetSelectionMark();
	if(index==-1)
	{
		MessageBox("Select a button first.");
		return;
	}

	char btn[256];
	m_conf.GetItemText(index,0,btn,255);
	
	for(int i=0;i<our_buttons.GetSize();i++)
	{
		if(stricmp(our_buttons[i],btn)==0)
		{
			our_buttons.RemoveAt(i);
			our_actions.RemoveAt(i);
			m_conf.DeleteItem(index);
			m_conf.SetSelectionMark(index);
			GetDlgItem(IDAPPLY)->EnableWindow(TRUE);
			GotoDlgCtrl(GetDlgItem(IDC_CONF));
			return;
		}
	}
	MessageBox("Couldn't find that button, doh!");
}

void Cconfig::OnOK() 
{
	OnApply();
		
	CDialog::OnOK();
}

void Cconfig::OnApply() 
{
	app->buttons.RemoveAll();
	app->actions.RemoveAll();
	int i;
	for(i=0;i<our_buttons.GetSize();i++)
		app->buttons.Add(our_buttons[i]);
	for(i=0;i<our_actions.GetSize();i++)
		app->actions.Add(our_actions[i]);
	GetDlgItem(IDAPPLY)->EnableWindow(FALSE);
	app->WriteConfig();
}
