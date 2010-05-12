// SerialDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SerialDialog.h"
#include "Globals.h"


// SerialDialog dialog

IMPLEMENT_DYNAMIC(SerialDialog, CDialog)

SerialDialog::SerialDialog(CWnd* pParent /*=NULL*/)
	: CDialog(SerialDialog::IDD, pParent)
	, test(0)
{
	animax			= FALSE;
	deviceType		= -1;
	virtualPulse	= 0;
	transmitterPin	= -1;
	hardwareCarrier	= FALSE;
	inverted		= FALSE;
}

SerialDialog::~SerialDialog()
{
}

void SerialDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PORT, port);
	DDX_Control(pDX, IDC_SPEED, speed);
	DDX_Control(pDX, IDC_SENSE, sense);
	DDX_Radio(pDX, IDC_RADIORX, deviceType);
	DDX_Radio(pDX, IDC_RADIODTR, transmitterPin);
	DDX_Check(pDX, IDC_CHECKANIMAX, animax);
	DDX_Check(pDX, IDC_CHECKHARDCARRIER, hardwareCarrier);
	DDX_Check(pDX, IDC_INVERTED, inverted);
	DDX_Text(pDX, IDC_VIRTPULSE, virtualPulse);
	DDV_MinMaxInt(pDX, virtualPulse, 0, 16777215);
}


BEGIN_MESSAGE_MAP(SerialDialog, CDialog)
	ON_BN_CLICKED(IDC_RADIORX, &SerialDialog::OnBnClickedRadiorx)
	ON_BN_CLICKED(IDC_RADIODCD, &SerialDialog::OnBnClickedRadiodcd)
	ON_BN_CLICKED(IDOK, &SerialDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// SerialDialog message handlers

BOOL SerialDialog::OnInitDialog() {

	//===========
	CComboBox *p;
	int x;
	//===========

	CDialog::OnInitDialog();

	settings.loadSettings();

	p = (CComboBox *)GetDlgItem(IDC_PORT);
	x = p->FindStringExact(0,settings.port);

	if(x != CB_ERR) {
		p->SetCurSel(x);
	}
	else {
		p->SetCurSel(0);
	}

	p = (CComboBox *)GetDlgItem(IDC_SPEED);
	x = p->FindStringExact(0,settings.speed);

	if(x != CB_ERR) {
		p->SetCurSel(x);
	}
	else {
		p->SetCurSel(0);
	}

	((CComboBox *)GetDlgItem(IDC_SENSE))->SetCurSel(settings.sense+1);

	animax			= settings.animax;
	hardwareCarrier	= settings.transmitterType & HARDCARRIER;
	transmitterPin	= (settings.transmitterType & TXTRANSMITTER)>>1;
	inverted		= (settings.transmitterType & INVERTED)>>2;
	virtualPulse	= settings.virtualPulse;
	deviceType		= settings.deviceType;

	UpdateData(FALSE);

	OnBnClickedRadiorx();

	return TRUE;
}

void SerialDialog::OnBnClickedRadiorx()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_SENSE)->EnableWindow(deviceType);
	GetDlgItem(IDC_VIRTPULSE)->EnableWindow(!deviceType);
	GetDlgItem(IDC_CHECKANIMAX)->EnableWindow(deviceType);
}

void SerialDialog::OnBnClickedRadiodcd()
{
	OnBnClickedRadiorx();
}

void SerialDialog::OnBnClickedOk()
{
	//===========
	CString temp;
	//===========

	OnOK();

	settings.transmitterType = (inverted<<2)|(transmitterPin<<1)|hardwareCarrier;

	int sense=((CComboBox *)GetDlgItem(IDC_SENSE))->GetCurSel();
	if(sense>=1 && sense<=2) sense--;
	else sense=-1;
	settings.sense=sense;

	port.GetWindowText(temp);
	settings.port			= temp;
	speed.GetWindowText(temp);
	settings.speed			= temp;	
	settings.deviceType		= deviceType;					
	settings.virtualPulse	= virtualPulse;
	settings.animax			= animax;

	settings.saveSettings();
}
