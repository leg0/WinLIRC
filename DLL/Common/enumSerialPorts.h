#pragma once

#ifndef _WINDOWS_
    #error This header requires Windows.h to be included.
#endif

#include <tchar.h>
#include <cassert>
#include <cstring>
#include <vector>

inline size_t StrLen(char const* s) { return strlen(s); }
inline size_t StrLen(wchar_t const* s) { return wcslen(s); }
inline int Str_N_Cmp(char const* s1, char const* s2, size_t n) { return strncmp(s1, s2, n); }
inline int Str_N_Cmp(wchar_t const* s1, wchar_t const* s2, size_t n) { return wcsncmp(s1, s2, n); }

template <typename Char>
Char const* str_literal(char const*, wchar_t const*) = delete;
template <> constexpr char const* str_literal<char>(char const* s, wchar_t const*) noexcept { return s; }
template <> constexpr wchar_t const* str_literal<wchar_t>(char const*, wchar_t const* s) noexcept { return s; }

inline DWORD query_dos_device(char const* devicename, char* buf, DWORD bufSize) noexcept { return QueryDosDeviceA(devicename, buf, bufSize); }
inline DWORD query_dos_device(wchar_t const* devicename, wchar_t* buf, DWORD bufSize) noexcept { return QueryDosDeviceW(devicename, buf, bufSize); }
#define LITERAL(Type, s) str_literal<Type>(s, L ## s)

template <typename Char, typename Fn>
inline void enumSerialPorts(Fn fn)
{
    std::vector<Char> devices(1024, 0);
    while (devices.size() < 1024 * 1024) // guard against allocating too much memory
    {
        DWORD size = query_dos_device(nullptr, &devices[0], devices.size());
        if (size == 0 && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            devices.resize(devices.size() * 2);
        else
            break;
    }

    Char const* s = &devices[0];
    while (StrLen(s) > 0)
    {
        if (Str_N_Cmp(LITERAL(Char, "COM"), s, 3) == 0)
        {
            fn(s);
        }
        s += StrLen(s) + 1;
    }
}

/// @param hwnd - handle of a window that contains the combo box to add detected COM ports to.
/// @param idcCombo - control ID of the combo box.
inline void enumSerialPorts(HWND hwnd, int idcCombo)
{
    assert(::IsWindow(hwnd));
    enumSerialPorts<TCHAR>([=](LPCTSTR s) {
        SendDlgItemMessage(hwnd, idcCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
    });
}

#undef LITERAL
