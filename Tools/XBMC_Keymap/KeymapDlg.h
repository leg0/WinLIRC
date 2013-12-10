
// KeymapDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "xbmcclient.h"

// CKeymapDlg dialog
class CKeymapDlg : public CDialog
{
// Construction
public:
	CKeymapDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_KEYMAP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLvnKeydownList1(NMHDR *pNMHDR, LRESULT *pResult);
	BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
private:
	void SetupListControl();
	BOOL SetupWinsock();
	BOOL ConnectToWinLIRC();
	void ShutdownWinsock();
	void WinLIRCMessage(WPARAM wParam, LPARAM lParam);
	void LBUTTONDOWNMessage(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WinLIRCData();
	void SaveSettings();
	void LoadSettings();

	CListCtrl m_listCtrl;
	BOOL m_winlircConnected;
	BOOL m_xbmcHello;			// udp so we don't really connect but we need to say hello once
	UINT_PTR m_timerID;
	SOCKET	m_socket;
	CStatic m_staticConnected;
	CXBMCClient m_xbmcClient;
public:
	afx_msg void OnBnClickedSaveSettings();
};
