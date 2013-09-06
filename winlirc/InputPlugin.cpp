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
	m_hasGuiFunction		= NULL;
	m_loadSetupGuiFunction	= NULL;
	m_dllFile				= NULL;
}

InputPlugin::~InputPlugin()
{
}

void InputPlugin::listDllFiles() {

	//==========================
	CFileFind	cFileFind;
	CString		searchFile;
	BOOL		found;
	BOOL		foundMatch;			// match with the ini config
	bool		canRecord;
	CString		temp;
	int			i;
	int			matchIndex;
	//==========================

	searchFile	= _T(".\\*.dll");
	found		= cFileFind.FindFile(searchFile);
	foundMatch	= FALSE;
	i			= 0;
	matchIndex	= 0;
	canRecord	= false;

	if(!found) {

		MessageBox(_T("No valid dlls found."));

		return;
	}

	while(found) {

		found = cFileFind.FindNextFile();

		if(checkDllFile(cFileFind.GetFilePath())) {

			m_cboxInputPlugin.AddString(cFileFind.GetFileName());
		
			if(cFileFind.GetFileName() == config.plugin) {
				m_cboxInputPlugin.SetCurSel(i);
				foundMatch	= TRUE;
				matchIndex	= i;
				canRecord	= checkRecording(cFileFind.GetFilePath());
			}
			
			i++;
		}
	}

	m_cboxInputPlugin.SetCurSel(matchIndex);
	m_cboxInputPlugin.GetLBText(matchIndex,temp);

	temp = _T(".\\") + temp;

	loadDll(temp);
	enableWindows(canRecord);
}

bool InputPlugin::checkDllFile(CString file) {

	//==========
	HMODULE tmp;
	//==========

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

bool InputPlugin::checkRecording(CString file) {

	//==========
	HMODULE tmp;
	//==========

	tmp = LoadLibrary(file);

	if(!tmp) return false;

	if(!GetProcAddress(tmp,_T("getHardware"))) { 
		FreeLibrary(tmp); return false; 
	}

	FreeLibrary(tmp);

	return true;
}

void InputPlugin::enableWindows(bool canRecord) {

	if(m_hasGuiFunction) {
		m_setupButton.EnableWindow(m_hasGuiFunction());
	}
	else {
		m_setupButton.EnableWindow(FALSE);
	}

	if(canRecord) {
		m_configPath.EnableWindow();
		m_createConfigButton.EnableWindow();
		m_browseButton.EnableWindow();
	}
	else {
		m_configPath.EnableWindow(FALSE);
		m_createConfigButton.EnableWindow(FALSE);
		m_browseButton.EnableWindow(FALSE);
	}

}

void InputPlugin::loadDll(CString file) {

	m_dllFile = LoadLibrary(file);

	if(!m_dllFile) return;

	m_hasGuiFunction		= (HasGuiFunction)			GetProcAddress(m_dllFile,_T("hasGui"));
	m_loadSetupGuiFunction	= (LoadSetupGuiFunction)	GetProcAddress(m_dllFile,_T("loadSetupGui"));
}

void InputPlugin::unloadDll() {

	//
	// make sure we have cleaned up
	//
	m_hasGuiFunction		= NULL;
	m_loadSetupGuiFunction	= NULL;

	FreeLibrary(m_dllFile);

	m_dllFile				= NULL;
}


void InputPlugin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_cboxInputPlugin);
	DDX_Control(pDX, IDC_BUTTON1, m_setupButton);
	DDX_Control(pDX, IDC_EDIT1, m_configPath);
	DDX_Control(pDX, IDC_CHECK1, m_disableKeyRepeats);
	DDX_Control(pDX, IDC_EDIT3, m_disableFirstRepeats);
	DDX_Control(pDX, IDC_DISABLE_FIRST_REPEATS, m_disableFirstRepeatsLabel);
	DDX_Control(pDX, IDC_CHECK2, m_allowLocalConnectionsOnly);
	DDX_Control(pDX, IDC_CHECK3, m_disableSystemTrayIcon);
	DDX_Control(pDX, IDC_BUTTON3, m_createConfigButton);
	DDX_Control(pDX, IDC_BUTTON2, m_browseButton);
}

BEGIN_MESSAGE_MAP(InputPlugin, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &InputPlugin::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDOK, &InputPlugin::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON2, &InputPlugin::OnBnClickedButton2)
	ON_BN_CLICKED(IDCANCEL, &InputPlugin::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &InputPlugin::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK1, &InputPlugin::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON3, &InputPlugin::OnBnClickedButton3)
END_MESSAGE_MAP()


// InputPlugin message handlers

void InputPlugin::OnCbnSelchangeCombo1() {

	//======================
	int		cursorSelection;
	CString file;
	bool	validFile;
	bool	canRecord;
	//======================

	unloadDll();

	cursorSelection = m_cboxInputPlugin.GetCurSel();

	m_cboxInputPlugin.GetLBText(m_cboxInputPlugin.GetCurSel(),file);

	file		= _T(".\\") + file;
	validFile	= checkDllFile(file);
	canRecord	= checkRecording(file);

	if(!validFile) MessageBox(_T("Invalid dll file"),_T("Error"),0);

	loadDll(file);
	enableWindows(canRecord);
}

void InputPlugin::OnBnClickedOk() {

	//=================
	CString confPath;
	CString fKeyReps;
	//=================

	m_configPath.GetWindowText(confPath);

	//
	// some basic error checking
	//

	if(confPath!="") {

		//========
		FILE *tmp;
		//========

		tmp = fopen(confPath,"r");

		if(tmp==NULL) {
			MessageBox(	"The configuration filename is invalid.\n"
				"Please try again.","Configuration Error");
			return;
		}
		else {
			fclose(tmp);
		}
	}

	config.remoteConfig = confPath;

	m_cboxInputPlugin.GetWindowText(config.plugin);

	if(m_disableKeyRepeats.GetCheck()==BST_CHECKED) {
		config.disableRepeats = TRUE;
	}
	else {
		config.disableRepeats = FALSE;
	}

	m_disableFirstRepeats.GetWindowText(fKeyReps);

	if(fKeyReps!="") {
		config.disableFirstKeyRepeats = _tstoi(fKeyReps);
	}
	else {
		config.disableFirstKeyRepeats = 0;
	}

	if(m_allowLocalConnectionsOnly.GetCheck()==BST_CHECKED) {
		config.localConnectionsOnly = TRUE;
	}
	else {
		config.localConnectionsOnly = FALSE;
	}

	if(m_disableSystemTrayIcon.GetCheck()==BST_CHECKED) {
		config.showTrayIcon = FALSE;
	}
	else {
		config.showTrayIcon = TRUE;
	}

	config.writeINIFile();

	OnOK();
}

void InputPlugin::OnBnClickedButton2() {

	CFileDialog fileDlg(TRUE,NULL,NULL,OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_ENABLESIZING,NULL,this,0,TRUE);

	if( fileDlg.DoModal ()==IDOK ) {
		m_configPath.SetWindowText(fileDlg.GetPathName());
	}
}

void InputPlugin::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void InputPlugin::OnBnClickedButton1()
{
	if(m_loadSetupGuiFunction) {

		this->EnableWindow(FALSE);
		m_loadSetupGuiFunction();
		this->EnableWindow(TRUE);
		this->SetFocus();
	}
}

BOOL InputPlugin::OnInitDialog() {

	//===========
	CString temp;
	//===========

	CDialog::OnInitDialog();

	listDllFiles();

	m_configPath.SetWindowText(config.remoteConfig);

	temp.Format("%i",config.disableFirstKeyRepeats);

	m_disableFirstRepeats.SetWindowText(temp);

	if(config.disableRepeats) {
		m_disableKeyRepeats.SetCheck(BST_CHECKED);
		m_disableFirstRepeats.EnableWindow(FALSE);
		m_disableFirstRepeatsLabel.EnableWindow(FALSE);
	}

	if(config.localConnectionsOnly) {
		m_allowLocalConnectionsOnly.SetCheck(BST_CHECKED);
	}

	if(!config.showTrayIcon) {
		m_disableSystemTrayIcon.SetCheck(BST_CHECKED);
	}
   
	return TRUE;

}

void InputPlugin::OnBnClickedCheck1() {

	//=============
	INT	checkState;
	//=============

	checkState = m_disableKeyRepeats.GetCheck();

	if(checkState==BST_CHECKED) {
		m_disableFirstRepeats.EnableWindow(FALSE);
		m_disableFirstRepeatsLabel.EnableWindow(FALSE);
	}
	else {
		m_disableFirstRepeats.EnableWindow(TRUE);
		m_disableFirstRepeatsLabel.EnableWindow(TRUE);
	}

}

void InputPlugin::OnBnClickedButton3() {
	
	//==============================
	CString				confPath;
	CString				pluginName;
	CString				commandLine;
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;
	BOOL				processCreated;
	//==============================

	m_configPath.GetWindowText(confPath);

	if(confPath==_T("")) {
		m_configPath.SetWindowText(_T("..\\config.cf"));
		m_configPath.GetWindowText(confPath);
	}

	m_cboxInputPlugin.GetWindowText(pluginName);

	if(pluginName==_T("")) {
		MessageBox(_T("No valid plugins selected."));
		return;
	}

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	commandLine = _T("..\\IRRecord -d ") + pluginName + _T(" \"") + confPath + _T("\"");

	processCreated = CreateProcess( NULL,	// No module name (use command line)
		commandLine.GetBuffer(0),			// Command line
		NULL,								// Process handle not inheritable
		NULL,								// Thread handle not inheritable
		FALSE,								// Set handle inheritance to FALSE
		0,									// No creation flags
		NULL,								// Use parent's environment block
		NULL,								// Use parent's starting directory 
		&si,								// Pointer to STARTUPINFO structure
		&pi );								// Pointer to PROCESS_INFORMATION structure

	if(processCreated) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		MessageBox(_T("IRRecord.exe missing"));
	}

}
