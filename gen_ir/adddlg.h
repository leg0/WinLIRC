#if !defined(AFX_ADDDLG_H__71CF5EC3_CDC6_11D2_8C7F_004005637418__INCLUDED_)
#define AFX_ADDDLG_H__71CF5EC3_CDC6_11D2_8C7F_004005637418__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDlg.h : header file
//

#include "config.h"

/////////////////////////////////////////////////////////////////////////////
// CAddDlg dialog

class CAddDlg : public CDialog
{
// Construction
public:
	CAddDlg(Cconfig *nconf, CWnd* pParent = NULL);   // standard constructor

	Cconfig *conf;

// Dialog Data
	//{{AFX_DATA(CAddDlg)
	enum { IDD = IDD_ADD };
	CComboBox	m_act;
	CString	m_id;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDLG_H__71CF5EC3_CDC6_11D2_8C7F_004005637418__INCLUDED_)
