#pragma once
#include "resource.h"
#include "Settings.h"

namespace irtiny
{
    class ConfigDialog
        : public CDialogImpl<ConfigDialog>
        , public CWinDataExchange<ConfigDialog>
    {
    public:
        ConfigDialog();

        enum { IDD = IDD_CONFIG_DIALOG };

    private:
        LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
        LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

        BEGIN_MSG_MAP(ConfigDialog)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_ID_HANDLER(IDOK, OnOK)
            COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        END_MSG_MAP()

        BEGIN_DDX_MAP(SerialDialog)
            DDX_CONTROL_HANDLE(IDC_PORT, cmbPort_)
            DDX_TEXT(IDC_PORT, strPortName_)
        END_DDX_MAP()

        CComboBox cmbPort_;
        CString strPortName_;

        Settings settings_;
    };
}
