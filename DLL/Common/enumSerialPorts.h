#pragma once

#ifndef _WINDOWS_
    #error This header requires Windows.h to be included.
#endif

#include <tchar.h>
#include <cassert>
#include <cstring>
#include <vector>

/// @param hwnd - handle of a window that contains the combo box to add detected COM ports to.
/// @param idcCombo - control ID of the combo box.
inline void enumSerialPorts(HWND hwnd, int idcCombo)
{
    assert(::IsWindow(hwnd));

    std::vector<TCHAR> devices(1024, 0);
    while (devices.size() < 1024 * 1024) // guard against allocating too much memory
    {
        DWORD size = ::QueryDosDevice(NULL, &devices[0], devices.size());
        if (size == 0 && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            devices.resize(devices.size() * 2);
        else
            break;
    }

    LPCTSTR s = &devices[0];
    while (_tcslen(s) > 0)
    {
        if (_tcsncmp(_T("COM"), s, 3) == 0)
        {
            SendDlgItemMessage(hwnd, idcCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
        }
        s += _tcslen(s) + 1;
    }
}
