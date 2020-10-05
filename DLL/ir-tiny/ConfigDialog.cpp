#include "stdafx.h"
#include "ConfigDialog.h"
#include "Common/enumSerialPorts.h"

// SerialDialog dialog

irtiny::ConfigDialog::ConfigDialog()
{ }

LRESULT irtiny::ConfigDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    DoDataExchange(FALSE);

    settings_.load();

    ::enumSerialPorts(*this, IDC_PORT);
    auto const x = cmbPort_.FindStringExact(0, settings_.port().c_str());
    cmbPort_.SetCurSel(x == CB_ERR ? 0 : x);

    DoDataExchange(FALSE);
    return TRUE;
}

LRESULT irtiny::ConfigDialog::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    DoDataExchange(TRUE);
    settings_.port(static_cast<wchar_t const*>(strPortName_));
    settings_.save();

    EndDialog(wID);
    return 0;
}

LRESULT irtiny::ConfigDialog::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}
