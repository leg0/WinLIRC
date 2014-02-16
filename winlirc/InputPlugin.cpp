// InputPlugin.cpp : implementation file
//

#include "stdafx.h"
#include "winlirc.h"
#include "InputPlugin.h"
#include "irconfig.h"
#include <atlbase.h>
#include "Server.h"


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

	if(!GetProcAddress(tmp,"init"))			{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,"deinit"))		{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,"hasGui"))		{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,"loadSetupGui")) { FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,"sendIR"))		{ FreeLibrary(tmp); return false; }
	if(!GetProcAddress(tmp,"decodeIR"))		{ FreeLibrary(tmp); return false; }

	FreeLibrary(tmp);

	return true;
}

bool InputPlugin::checkRecording(CString file) {

	//==========
	HMODULE tmp;
	//==========

	tmp = LoadLibrary(file);

	if(!tmp) return false;

	if(!GetProcAddress(tmp,"getHardware")) { 
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

	m_hasGuiFunction		= (HasGuiFunction)			GetProcAddress(m_dllFile,"hasGui");
	m_loadSetupGuiFunction	= (LoadSetupGuiFunction)	GetProcAddress(m_dllFile,"loadSetupGui");
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
	DDX_Control(pDX, IDC_CMB_INPUT_PLUGIN, m_cboxInputPlugin);
	DDX_Control(pDX, IDC_BN_PLUGIN_SETUP, m_setupButton);
	DDX_Control(pDX, IDC_EDIT_CONFIG_PATH, m_configPath);
	DDX_Control(pDX, IDC_CHK_DISABLE_KEY_REPEATS, m_disableKeyRepeats);
	DDX_Control(pDX, IDC_EDIT_DISABLE_FIRST_REPEATS, m_disableFirstRepeats);
	DDX_Control(pDX, IDC_DISABLE_FIRST_REPEATS, m_disableFirstRepeatsLabel);
	DDX_Control(pDX, IDC_CHK_LOCAL_CONNECTIONS_ONLY, m_allowLocalConnectionsOnly);
	DDX_Control(pDX, IDC_CHK_DISABLE_TRAY_ICON, m_disableSystemTrayIcon);
	DDX_Control(pDX, IDC_BN_CREATE_CONFIG, m_createConfigButton);
	DDX_Control(pDX, IDC_BN_BROWSE, m_browseButton);
	DDX_Control(pDX, IDC_CHK_START_WITH_WINDOWS, m_startWithWindows);
	DDX_Control(pDX, IDC_EDIT2, m_editDefaultPort);
}

BEGIN_MESSAGE_MAP(InputPlugin, CDialog)
	ON_CBN_SELCHANGE(IDC_CMB_INPUT_PLUGIN, &InputPlugin::OnCbnSelchangeInputPlugin)
	ON_BN_CLICKED(IDOK, &InputPlugin::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BN_BROWSE, &InputPlugin::OnBnClickedBrowse)
	ON_BN_CLICKED(IDCANCEL, &InputPlugin::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BN_PLUGIN_SETUP, &InputPlugin::OnBnClickedPluginSetup)
	ON_BN_CLICKED(IDC_CHK_DISABLE_KEY_REPEATS, &InputPlugin::OnBnClickedDisableKeyRepeats)
	ON_BN_CLICKED(IDC_BN_CREATE_CONFIG, &InputPlugin::OnBnClickedCreateConfig)
END_MESSAGE_MAP()


// InputPlugin message handlers

void InputPlugin::OnCbnSelchangeInputPlugin() {

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
	CString tempString;
	//=================

	m_configPath.GetWindowText(confPath);

	//
	// some basic error checking
	//

	if(confPath!="") {

		//========
		FILE *tmp;
		//========

		tmp = _tfopen(confPath,_T("r"));

		if(tmp==NULL) {
			MessageBox(	_T("The configuration filename is invalid.\n")
				_T("Please try again."),_T("Configuration Error"));
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

	m_disableFirstRepeats.GetWindowText(tempString);

	if(tempString!="") {
		config.disableFirstKeyRepeats = _tstoi(tempString);
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

	{
		//===========
		int tempPort;
		//===========

		m_editDefaultPort.GetWindowText(tempString);

		if(tempString!="")	{ tempPort = _tstoi(tempString); }
		else				{ tempPort = 8765; }

		if(tempPort!=config.serverPort) {

			//===========
			bool success;
			//===========

			config.serverPort = tempPort;
			success = app.server->restartServer();

			if(!success) {
				MessageBox(_T("Server could not be started. Try checking the port."),_T("WinLIRC"),MB_OK|MB_ICONERROR);
				return;
			}
		}
	}

	setStartup(m_startWithWindows.GetCheck() == BST_CHECKED);

	config.writeINIFile();

	OnOK();
}

void InputPlugin::OnBnClickedBrowse() {

	CFileDialog fileDlg(TRUE,NULL,NULL,OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_ENABLESIZING,NULL,this,0,TRUE);

	if( fileDlg.DoModal ()==IDOK ) {
		m_configPath.SetWindowText(fileDlg.GetPathName());
	}
}

void InputPlugin::OnBnClickedCancel()
{
	OnCancel();
}

void InputPlugin::OnBnClickedPluginSetup()
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

	temp.Format(_T("%i"),config.disableFirstKeyRepeats);
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

	if(getStartup()) {
		m_startWithWindows.SetCheck(BST_CHECKED);
	}

	temp.Format(_T("%i"),config.serverPort);
	m_editDefaultPort.SetWindowText(temp);
   
	return TRUE;

}

void InputPlugin::OnBnClickedDisableKeyRepeats() {

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

void InputPlugin::OnBnClickedCreateConfig() {
	
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

bool InputPlugin::getStartup() {

	//==========
	CRegKey	key;
	//==========

	if(key.Open(HKEY_CURRENT_USER,_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),KEY_READ)==ERROR_SUCCESS) {

		//===================
		TCHAR path[MAX_PATH];
		ULONG pathLength;
		//===================

		pathLength = _countof(path);

		if(key.QueryStringValue(_T("WinLIRC"),path,&pathLength)==ERROR_SUCCESS) {
			return true;
		}
	}

	return false;
}

void InputPlugin::setStartup(bool start) {

	//==========
	CRegKey	key;
	//==========

	if(key.Open(HKEY_CURRENT_USER,_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"))==ERROR_SUCCESS) {

		if(start) {

			//=========================
			TCHAR modulePath[MAX_PATH];
			//=========================

			GetModuleFileName(NULL,modulePath,_countof(modulePath));

			key.SetStringValue(_T("WinLIRC"),modulePath);
		}
		else {
			key.DeleteValue(_T("WinLIRC"));
		}
	}
}

