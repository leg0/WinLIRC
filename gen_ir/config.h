#if !defined(AFX_CONFIG_H__DD1219A3_CCBA_11D2_8C7F_004005637418__INCLUDED_)
#define AFX_CONFIG_H__DD1219A3_CCBA_11D2_8C7F_004005637418__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// config.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Cconfig dialog

class CGen_irApp;
class Cconfig : public CDialog
{
// Construction
public:
	Cconfig(CGen_irApp *napp, CWnd* pParent = NULL);   // standard constructor

	CGen_irApp *app;

	CStringArray our_buttons;
	CDWordArray our_actions;

// Dialog Data
	//{{AFX_DATA(Cconfig)
	enum { IDD = IDD_CONFIG };
	CListCtrl	m_conf;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cconfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void RefreshList(void);

	// Generated message map functions
	//{{AFX_MSG(Cconfig)
	afx_msg void OnClickConf(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	virtual void OnOK();
	afx_msg void OnApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIG_H__DD1219A3_CCBA_11D2_8C7F_004005637418__INCLUDED_)
