// InputPlugin.cpp : implementation file
//

#include "stdafx.h"
#include "winlirc.h"
#include "InputPlugin.h"
#include "irconfig.h"
#include <atlbase.h>
#include "Server.h"

#include <cassert>
#include <filesystem>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

IMPLEMENT_DYNAMIC(InputPlugin, CDialog)

InputPlugin::InputPlugin(CWnd* pParent /*=nullptr*/)
	: CDialog(InputPlugin::IDD, pParent)
{ }

fs::path getPluginsDirectory();

void InputPlugin::listDllFiles()
{
	std::vector<fs::path> plugins;
	auto pluginsDir = getPluginsDirectory();
	for (auto& p : fs::directory_iterator(pluginsDir))
	{
		auto path = p.path();
		if (path.extension() == L".dll")
		{
			Plugin plugin{ path };
			if (plugin.hasValidInterface())
				plugins.push_back(absolute(path));
		}
	}

	if (plugins.empty())
	{
		MessageBox(_T("No valid dlls found."));
		m_plugins.clear();
		return;
	}

	std::sort(begin(plugins), end(plugins));
	for (auto const& filePath : plugins)
	{
		m_cboxInputPlugin.AddString(filePath.filename().wstring().c_str());
	}

	auto currentPlugin = std::find(begin(plugins), end(plugins), config.plugin);
	if (currentPlugin == end(plugins))
		currentPlugin = begin(plugins);

	m_cboxInputPlugin.SetCurSel(std::distance(begin(plugins), currentPlugin));

	loadDll(*currentPlugin);
	enableWindows(m_plugin.canRecord());
	m_plugins = std::move(plugins);
}

void InputPlugin::enableWindows(bool canRecord) {

	auto hasGuiFn = m_plugin.interface_.hasGui;
	m_setupButton.EnableWindow(hasGuiFn && hasGuiFn());
	m_configPath.EnableWindow(canRecord);
	m_createConfigButton.EnableWindow(canRecord);
	m_browseButton.EnableWindow(canRecord);
}

void InputPlugin::loadDll(std::wstring const& file)
{
	if (Plugin plugin{ file.c_str() })
		m_plugin = std::move(plugin);
}

void InputPlugin::unloadDll()
{
	m_plugin = Plugin{ };
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

void InputPlugin::OnCbnSelchangeInputPlugin()
{
	unloadDll();

	auto const selection = m_cboxInputPlugin.GetCurSel();
	if (selection == CB_ERR)
		return;

	assert(selection < m_plugins.size());
	auto const& file = m_plugins[selection];

	Plugin plugin{ file };
	if (plugin.hasValidInterface())
	{
		m_plugin = std::move(plugin);
		enableWindows(m_plugin.canRecord());
	}
	else
	{
		MessageBoxW(L"Invalid dll file", L"Error", 0);
	}
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

		if(tmp==nullptr) {
			MessageBox(	_T("The configuration filename is invalid.\n")
				_T("Please try again."),_T("Configuration Error"));
			return;
		}
		else {
			fclose(tmp);
		}
	}

	config.remoteConfig = confPath;

	config.plugin = m_plugins[m_cboxInputPlugin.GetCurSel()];

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
			success = app.server.restartServer();

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

	CFileDialog fileDlg(TRUE,nullptr,nullptr,OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_ENABLESIZING,nullptr,this,0,TRUE);

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
	if (auto loadSetupGui = m_plugin.interface_.loadSetupGui)
	{
		this->EnableWindow(FALSE);
		loadSetupGui();
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

	processCreated = CreateProcess( nullptr,	// No module name (use command line)
		commandLine.GetBuffer(0),			// Command line
		nullptr,								// Process handle not inheritable
		nullptr,								// Thread handle not inheritable
		FALSE,								// Set handle inheritance to FALSE
		0,									// No creation flags
		nullptr,								// Use parent's environment block
		nullptr,								// Use parent's starting directory 
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

			GetModuleFileName(nullptr,modulePath,_countof(modulePath));

			key.SetStringValue(_T("WinLIRC"),modulePath);
		}
		else {
			key.DeleteValue(_T("WinLIRC"));
		}
	}
}

