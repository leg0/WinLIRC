#include "stdafx.h"
#include "ConfigDialog.h"
#include "Common/enumSerialPorts.h"
#include "winlirc/winlirc_api.h"

LRESULT irtiny::ConfigDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    DoDataExchange(FALSE);

    wchar_t port[32] = L"";
    winlirc_settings_get_wstring(L"ir-tiny", L"port", port, std::size(port) - 1, L"");

    ::enumSerialPorts(*this, IDC_PORT);
    auto const x = cmbPort_.FindStringExact(0, port);
    cmbPort_.SetCurSel(x == CB_ERR ? 0 : x);

    DoDataExchange(FALSE);
    return TRUE;
}

LRESULT irtiny::ConfigDialog::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    DoDataExchange(TRUE);
    winlirc_settings_set_wstring(L"ir-tiny", L"port", strPortName_);

    EndDialog(wID);
    return 0;
}

LRESULT irtiny::ConfigDialog::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}
