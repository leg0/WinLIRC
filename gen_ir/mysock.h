#if !defined(AFX_MYSOCK_H__D55A1698_CC1B_11D2_8C7F_004005637418__INCLUDED_)
#define AFX_MYSOCK_H__D55A1698_CC1B_11D2_8C7F_004005637418__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// mysock.h : header file
//

class CGen_irApp;
#define MAXLEN 4096


/////////////////////////////////////////////////////////////////////////////
// CMySocket command target

class CMySocket : public CAsyncSocket
{
// Attributes
public:

// Operations
public:
	CMySocket();
	CGen_irApp *app;	
	virtual ~CMySocket();

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMySocket)
	public:
	virtual void OnConnect(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CMySocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:

	int curr;
	char buf[MAXLEN];
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSOCK_H__D55A1698_CC1B_11D2_8C7F_004005637418__INCLUDED_)
