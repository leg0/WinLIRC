#pragma once

#include "afxwin.h"
#include "Plugin.h"
#include <filesystem>
#include <string>
#include <vector>

class InputPlugin : public CDialog
{
	DECLARE_DYNAMIC(InputPlugin)

public:
	InputPlugin(CWnd* pParent = nullptr);

	enum { IDD = IDD_DIALOG1 };

private:
	void	listDllFiles	();
	void	enableWindows	(bool canRecord);					// enable windows based upon selection
	void	loadDll			(std::filesystem::path const& file);
	void	unloadDll		();
	bool	getStartup		();
	void	setStartup		(bool start);

	using HasGuiFunction = int(*)();
	using LoadSetupGuiFunction = void(*)();

	Plugin m_plugin;

protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	virtual BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()

private:

	afx_msg void OnCbnSelchangeInputPlugin();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedPluginSetup();
	afx_msg void OnBnClickedDisableKeyRepeats();
	afx_msg void OnBnClickedCreateConfig();

	CComboBox	m_cboxInputPlugin;
	CButton		m_setupButton;
	CEdit		m_configPath;
	CButton		m_disableKeyRepeats;
	CEdit		m_disableFirstRepeats;
	CStatic		m_disableFirstRepeatsLabel;
	CButton		m_allowLocalConnectionsOnly;
	CButton		m_disableSystemTrayIcon;	
	CButton		m_createConfigButton;
	CButton		m_browseButton;
	CButton		m_startWithWindows;
	CEdit		m_editDefaultPort;

	// contents of the combobox. same sort order.
	std::vector<std::filesystem::path> m_plugins;
};
