#pragma once
#include "afxwin.h"


// InputPlugin dialog

class InputPlugin : public CDialog
{
	DECLARE_DYNAMIC(InputPlugin)

public:
	InputPlugin(CWnd* pParent = NULL);   // standard constructor
	virtual ~InputPlugin();

	enum { IDD = IDD_DIALOG1 };

private:
	void listDllFiles	();
	bool checkDllFile	(CString file);
	void loadDll		(CString file);
	void unloadDll		();

	typedef int  (*HasGuiFunction)			();
	typedef void (*LoadSetupGuiFunction)	();

	HasGuiFunction			hasGuiFunction;
	LoadSetupGuiFunction	loadSetupGuiFunction;

	HMODULE					dllFile;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
private:
	CComboBox cboxInputPlugin;
	CButton setupButton;
	CEdit configPath;
	CButton disableKeyRepeats;
};
