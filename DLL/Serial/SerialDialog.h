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
	int test;
	DECLARE_MESSAGE_MAP()
};
