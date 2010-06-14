// InputPlugin.cpp : implementation file
//

#include "stdafx.h"
#include "winlirc.h"
#include "InputPlugin.h"
#include "irconfig.h"


// InputPlugin dialog

IMPLEMENT_DYNAMIC(InputPlugin, CDialog)

InputPlugin::InputPlugin(CWnd* pParent /*=NULL*/)
	: CDialog(InputPlugin::IDD, pParent)
{
	hasGuiFunction			= NULL;
	loadSetupGuiFunction	= NULL;

	dllFile					= NULL;
}

InputPlugin::~InputPlugin()
{
	//printf("window closed and died :(\n");
}

void InputPlugin::listDllFiles() {

	//==========================
	CFileFind	cFileFind;
	CString		searchFile;
	BOOL		found;
	BOOL		foundMatch;			// match with the ini config
	CString		temp;
	int			i;
	int			matchIndex;
	//==========================

	searchFile	= _T(".\\*.dll");
	found		= cFileFind.FindFile(searchFile);
	foundMatch	= FALSE;
	i			= 0;
	matchIndex	= 0;

	if(!found) {

		MessageBox(_T("No valid dlls found."));

		return;
	}

	while(found) {

		found = cFileFind.FindNextFile();

		if(checkDllFile(cFileFind.GetFilePath())) {

			cboxInputPlugin.AddString(cFileFind.GetFileName());
		
			if(cFileFind.GetFileName() == config.plugin) {
				cboxInputPlugin.SetCurSel(i);
				foundMatch = TRUE;
				matchIndex = i;
			}
			
			i++;
		}
	}

	cboxInputPlugin.SetCurSel(matchIndex);
	cboxInputPlugin.GetLBText(matchIndex,temp);

	temp = _T(".\\") + temp;

	loadDll(temp);

	if(hasGuiFunction) {
		setupButton.EnableWindow(hasGuiFunction());
	}
	else {
		//printf("failed :( %s \n",temp);
		setupButton.EnableWindow(FALSE);
	}
}

bool InputPlugin::checkDllFile(CString file) {

	//==============
	HMODULE tmp;
	//==============

	tmp = LoadLibrary(file);

	if(!tmp) return false;

	if(!GetProcAddress(tmp,_T("init")))			{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,_T("deinit")))		{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,_T("hasGui")))		{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,_T("loadSetupGui"))) { FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,_T("sendIR")))		{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,_T("decodeIR")))		{ FreeLibrary(tmp); return false; }

	FreeLibrary(tmp);

	return true;
}

void InputPlugin::loadDll(CString file) {

	dllFile = LoadLibrary(file);

	if(!dllFile) return;

	hasGuiFunction			= (HasGuiFunction)		GetProcAddress(dllFile,_T("hasGui"));
	loadSetupGuiFunction	= (LoadSetupGuiFunction)GetProcAddress(dllFile,_T("loadSetupGui"));
}

void InputPlugin::unloadDll() {

	//
	// make sure we have cleaned up
	//
	hasGuiFunction			= NULL;
	loadSetupGuiFunction	= NULL;

	FreeLibrary(dllFile);

	dllFile					= NULL;
}


void InputPlugin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, cboxInputPlugin);
	DDX_Control(pDX, IDC_BUTTON1, setupButton);
	DDX_Control(pDX, IDC_EDIT1, configPath);
	DDX_Control(pDX, IDC_CHECK1, disableKeyRepeats);
}

BEGIN_MESSAGE_MAP(InputPlugin, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &InputPlugin::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDOK, &InputPlugin::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON2, &InputPlugin::OnBnClickedButton2)
	ON_BN_CLICKED(IDCANCEL, &InputPlugin::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &InputPlugin::OnBnClickedButton1)
//	ON_WM_CREATE()
END_MESSAGE_MAP()


// InputPlugin message handlers

void InputPlugin::OnCbnSelchangeCombo1() {

	//======================
	int		cursorSelection;
	CString file;
	//======================

	unloadDll();

	cursorSelection = cboxInputPlugin.GetCurSel();

	cboxInputPlugin.GetLBText(cboxInputPlugin.GetCurSel(),file);

	file = _T(".\\") + file;

	bool tmp = checkDllFile(file);

	//printf("%i worked \n",tmp);
	if(!tmp) MessageBox(_T("Invalid dll file"),_T("Error"),0);

	loadDll(file);

	if(hasGuiFunction) {
		setupButton.EnableWindow(hasGuiFunction());
	}
	else {
		setupButton.EnableWindow(FALSE);
	}


}

void InputPlugin::OnBnClickedOk() {

	//=================
	CString confPath;
	INT		checkState;
	//=================

	configPath.GetWindowText(confPath);
	config.remoteConfig = confPath;

	cboxInputPlugin.GetWindowText(config.plugin);

	checkState = disableKeyRepeats.GetCheck();

	if(checkState==BST_CHECKED) {
		config.disableRepeats = TRUE;
	}
	else {
		config.disableRepeats = FALSE;
	}

	config.writeConfig();

	OnOK();
}

void InputPlugin::OnBnClickedButton2() {

	CFileDialog fileDlg(TRUE,NULL,NULL,OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_ENABLESIZING,NULL,this,0,TRUE);

	if( fileDlg.DoModal ()==IDOK ) {
		configPath.SetWindowText(fileDlg.GetPathName());
	}
}

void InputPlugin::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void InputPlugin::OnBnClickedButton1()
{
	if(loadSetupGuiFunction) {

		this->EnableWindow(FALSE);
		loadSetupGuiFunction();
		this->EnableWindow(TRUE);
		this->SetFocus();
	}
}

BOOL InputPlugin::OnInitDialog() {

	CDialog::OnInitDialog();

	listDllFiles();

	configPath.SetWindowText(config.remoteConfig);

	if(config.disableRepeats) {
		disableKeyRepeats.SetCheck(BST_CHECKED);
	}
   
	return TRUE;

}
