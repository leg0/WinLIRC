
// KeymapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Keymap.h"
#include "KeymapDlg.h"
#include <tinyxml2.h>
#include <Shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WINLIRC_MESSAGE (WM_USER+1)

struct KeyMapTable
{
	LPCTSTR function;
	LPCTSTR key;
	CString lircKey;
};

KeyMapTable keyMapTable [] = {
	{_T("Up"),_T("up"),_T("")},
	{_T("Down"),_T("down"),_T("")},
	{_T("Left"),_T("left"),_T("")},
	{_T("Right"),_T("right"),_T("")},
	{_T("Play"),_T("p"),_T("")},
	{_T("Stop"),_T("x"),_T("")},
	{_T("Pause"),_T("p"),_T("")},
	{_T("FastForward"),_T("f"),_T("")},
	{_T("Rewind"),_T("r"),_T("")},
	{_T("Queue"),_T("q"),_T("")},
	{_T("PageUp"),_T("pageup"),_T("")},
	{_T("PageDown"),_T("pagedown"),_T("")},
	{_T("Select"),_T("enter"),_T("")},
	{_T("Back"),_T("backspace"),_T("")},
	{_T("PlayerControls"),_T("m"),_T("")},
	{_T("ShutdownMenu"),_T("s"),_T("")},
	{_T("PreviousMenu"),_T("escape"),_T("")},
	{_T("Info"),_T("i"),_T("")},
	{_T("ContextMenu"),_T("c"),_T("")},
	{_T("SkipNext"),_T("period"),_T("")},
	{_T("SkipPrevious"),_T("comma"),_T("")},
	{_T("FullScreen"),_T("tab"),_T("")},
	{_T("Screenshot"),_T("printscreen"),_T("")},
	{_T("VolumeDown"),_T("minus"),_T("")},
	{_T("VolumeUp"),_T("plus"),_T("")},
	{_T("Number0"),_T("zero"),_T("")},
	{_T("Number1"),_T("one"),_T("")},
	{_T("Number2"),_T("two"),_T("")},
	{_T("Number3"),_T("three"),_T("")},
	{_T("Number4"),_T("four"),_T("")},
	{_T("Number5"),_T("five"),_T("")},
	{_T("Number6"),_T("six"),_T("")},
	{_T("Number7"),_T("seven"),_T("")},
	{_T("Number8"),_T("eight"),_T("")},
	{_T("Number9"),_T("nine"),_T("")},
	{_T("ToggleFullScreen"),_T("backslash"),_T("")},
	{_T("FirstPage"),_T("home"),_T("")},
	{_T("LastPage"),_T("end"),_T("")}
};


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CKeymapDlg dialog




CKeymapDlg::CKeymapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKeymapDlg::IDD, pParent)
{
	m_hIcon				= AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_winlircConnected	= FALSE;
	m_xbmcHello			= FALSE;
	m_socket			= INVALID_SOCKET;

	LoadSettings();
	SetupWinsock();
}

void CKeymapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listCtrl);
	DDX_Control(pDX, IDC_STATIC_CONNECTED, m_staticConnected);
}

BEGIN_MESSAGE_MAP(CKeymapDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST1, &CKeymapDlg::OnLvnKeydownList1)
	ON_WM_NCLBUTTONDOWN()
	ON_BN_CLICKED(ID_SAVE_SETTINGS, &CKeymapDlg::OnBnClickedSaveSettings)
END_MESSAGE_MAP()


// CKeymapDlg message handlers

BOOL CKeymapDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	SetupListControl();
	m_staticConnected.EnableWindow(FALSE);

	m_timerID = SetTimer(1,2000,NULL);

	ShowWindow(SW_MINIMIZE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CKeymapDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CKeymapDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CKeymapDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CKeymapDlg::SetupListControl() 
{
	// setup columns
	LVCOLUMN lvColumn;

	lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 100;
	lvColumn.pszText = const_cast<LPTSTR>(_T("XBMC Action"));

	m_listCtrl.InsertColumn(0, &lvColumn);

	/*
	lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 100;
	lvColumn.pszText = _T("XBMC Key");

	m_listCtrl.InsertColumn(1, &lvColumn);
	*/

	lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 100;
	lvColumn.pszText = const_cast<LPTSTR>(_T("LIRC Key"));

	m_listCtrl.InsertColumn(1, &lvColumn);

	// add row data

	m_listCtrl.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	for(int i=0; i<_countof(keyMapTable); i++) 
	{
		//=============
		LVITEM	lvItem;
		int		nItem;
		//=============

		lvItem.mask		= LVIF_TEXT;
		lvItem.iItem	= i;
		lvItem.iSubItem = 0;
		lvItem.pszText	= (LPWSTR)keyMapTable[i].function;

		nItem			= m_listCtrl.InsertItem(&lvItem);

		//m_listCtrl.SetItemText(nItem, 1, keyMapTable[i].key);
		m_listCtrl.SetItemText(nItem, 1, keyMapTable[i].lircKey);
	}

}

BOOL CKeymapDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg)
	{
		if(pMsg->message==WINLIRC_MESSAGE)
		{
			WinLIRCMessage(pMsg->wParam,pMsg->lParam);
		}
		if(pMsg->message==WM_LBUTTONDOWN)
		{
			LBUTTONDOWNMessage(pMsg->hwnd,pMsg->wParam,pMsg->lParam);
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CKeymapDlg::WinLIRCMessage(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case FD_ACCEPT:
		break;
	case FD_CONNECT:
		m_winlircConnected = TRUE;
		m_staticConnected.EnableWindow(TRUE);
		break;
	case FD_READ:
		WinLIRCData();
		break;
	case FD_CLOSE:
		m_winlircConnected = FALSE;
		m_staticConnected.EnableWindow(FALSE);

		if(m_socket!=INVALID_SOCKET) 
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
		break;
	}
}

void CKeymapDlg::LBUTTONDOWNMessage(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if(hWnd==this->m_hWnd)
	{
		POSITION p = m_listCtrl.GetFirstSelectedItemPosition();
		
		if(p)
		{
			USES_CONVERSION;
			int nSelected = m_listCtrl.GetNextSelectedItem(p);

			m_listCtrl.SetItemState(nSelected, ~LVIS_SELECTED, LVIS_SELECTED);
			m_listCtrl.SetItemState(nSelected, ~LVIS_FOCUSED, LVIS_FOCUSED);
		}

	}
}

void CKeymapDlg::WinLIRCData()
{
	//======================
	CHAR	buffer[1024];
	INT		result;
	INT64	keycode;
	UINT	repeatCount;
	CHAR	command[128];
	CHAR	remoteName[128];
	//======================

	ZeroMemory(buffer,sizeof(buffer));	//clear memory

	result = recv(m_socket,buffer,sizeof(buffer)-1,0);

	if(result <= 0) {
		return;							//connection closed .. or error
	}

	if(sscanf_s(buffer,"%I64x %x %s %s",&keycode,&repeatCount,command,sizeof(command),remoteName,sizeof(remoteName))!=4) {
		return;
	}


	POSITION p = m_listCtrl.GetFirstSelectedItemPosition();
	
	if(p)
	{
		USES_CONVERSION;
		int nSelected = m_listCtrl.GetNextSelectedItem(p);

		m_listCtrl.SetItemText(nSelected, 1, A2W(command));
		keyMapTable[nSelected].lircKey = command;
	}
	else 
	{
		if(!m_xbmcHello)
		{
			if(PathFileExists(_T(".\\WinLIRC_48.png")))
			{
				m_xbmcClient.SendHELO("WinLIRC", ICON_PNG, "WinLIRC_48.png");
			}

			m_xbmcHello = TRUE;
		}

		USES_CONVERSION;

		LPWSTR x = A2W(command);
		CString csCommand = x;

		for(int i=0; i<_countof(keyMapTable); i++)
		{
			if(csCommand==keyMapTable[i].lircKey)
			{
				m_xbmcClient.SendButton(W2A(keyMapTable[i].key), "KB", BTN_USE_NAME | BTN_DOWN | BTN_QUEUE);
				m_xbmcClient.SendButton(W2A(keyMapTable[i].key), "KB", BTN_USE_NAME | BTN_UP | BTN_QUEUE);
			}
		}	
	}

}

BOOL CKeymapDlg::SetupWinsock()
{
	//============
    WSADATA	w;					//Winsock startup info
	INT		error;
	//============
    
    error = WSAStartup (MAKEWORD(2, 2), &w);   // Fill in WSA info
     
    if (error) 
	{
		return FALSE;
	}

    if (w.wVersion != MAKEWORD(2, 2)) 
	{ 
		WSACleanup ();		// wrong WinSock version! unload ws2_32.dll
		return FALSE;
    }

	return TRUE;
}

void CKeymapDlg::ShutdownWinsock()
{
	WSACleanup();
}

afx_msg void CKeymapDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(m_winlircConnected==FALSE)
	{
		ConnectToWinLIRC();
	}
}

BOOL CKeymapDlg::ConnectToWinLIRC()
{
	//=================
	SOCKADDR_IN target;	
	//=================

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// Create socket

	if (m_socket == INVALID_SOCKET) {
		return FALSE;
	}

	target.sin_family		= AF_INET;					// address family Internet
	target.sin_port			= htons(8765);				// set server’s port number
	target.sin_addr.s_addr	= inet_addr("127.0.0.1");	// set server’s IP
     
	//Try connecting...

	WSAAsyncSelect (m_socket, this->m_hWnd, WINLIRC_MESSAGE, FD_READ | FD_CONNECT | FD_CLOSE); 

	if (connect(m_socket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) { 
		  return FALSE;
	}
          
	return TRUE;
}

void CKeymapDlg::SaveSettings()
{
	//========================
	tinyxml2::XMLDocument doc;
	//========================

	tinyxml2::XMLElement *root = doc.NewElement("Keymap");
	doc.InsertEndChild(root);

	for(int i=0; i<_countof(keyMapTable); i++) 
	{
		USES_CONVERSION;

		tinyxml2::XMLElement *element = doc.NewElement(W2A(keyMapTable[i].function));

		element->SetAttribute("k",W2A(keyMapTable[i].lircKey));
		//element->InsertFirstChild( doc.NewText(W2A(keyMapTable[i].lircKey)));
		root->InsertEndChild(element);
	}

	doc.SaveFile("keymap.xml");
}

void CKeymapDlg::LoadSettings()
{
	//=========================
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement *root;
	//=========================

	if(doc.LoadFile("keymap.xml")!=tinyxml2::XML_SUCCESS)
	{
		return;
	}

	root = doc.RootElement();

	for(int i=0; i<_countof(keyMapTable); i++) 
	{
		USES_CONVERSION;

		tinyxml2::XMLElement *element = root->FirstChildElement(W2A(keyMapTable[i].function));

		keyMapTable[i].lircKey = element->Attribute("k");
	}
}

void CKeymapDlg::OnLvnKeydownList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if(pLVKeyDow->wVKey==VK_DELETE||pLVKeyDow->wVKey==VK_BACK)
	{
		POSITION p = m_listCtrl.GetFirstSelectedItemPosition();
		
		if(p)
		{
			USES_CONVERSION;
			int nSelected = m_listCtrl.GetNextSelectedItem(p);

			m_listCtrl.SetItemText(nSelected, 1, _T(""));
			keyMapTable[nSelected].lircKey = "";
		}
	}
}

void CKeymapDlg::OnBnClickedSaveSettings()
{
	SaveSettings();
}
