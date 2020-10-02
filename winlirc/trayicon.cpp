/*
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Derived from published code by Paul DiLascia.
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#include "stdafx.h"
#include <afxpriv.h>
#include "trayicon.h"
#include "winlirc.h"
#include <iterator>

template <size_t Size>
static auto LoadString(UINT uID, wchar_t(&s)[Size]) noexcept
{
    return ::LoadStringW(::GetModuleHandle(nullptr), uID, s, Size);
}

CTrayIcon::CTrayIcon(UINT uID) noexcept
    : icondata{ 0 }
{
    icondata.cbSize = sizeof(icondata);
    icondata.uID = uID;
    ::LoadString(uID, icondata.szTip);
}

CTrayIcon::~CTrayIcon() noexcept
{
    DisableTrayIcon();
}

void CTrayIcon::SetNotificationWnd(CWnd* notifywnd, UINT message) noexcept
{
    if (notifywnd == nullptr || !::IsWindow(notifywnd->GetSafeHwnd()))
    {
        WL_DEBUG("Invalid window\n");
        return;
    }

    if (message != 0 && message < WM_USER)
    {
        WL_DEBUG("Invalid message\n");
        message = 0;
    }

    icondata.hWnd = notifywnd->GetSafeHwnd();
    icondata.uCallbackMessage = message;
}

bool CTrayIcon::SetIcon(UINT uID) noexcept
{
    if (uID)
    {
        ::LoadString(uID, icondata.szTip);
        auto const icon = AfxGetApp()->LoadIcon(uID);
        return SetIcon(icon, nullptr);
    }
    else
    {
        return DisableTrayIcon();
    }
}

bool CTrayIcon::SetIcon(HICON icon, wchar_t const* tip) noexcept
{
    if (icon == nullptr && icondata.hIcon == nullptr)
        // nothing to do here. already not present.
        return true;

    UINT msg;
    UINT uFlags = 0;

    if (icon)
    {
        msg = icondata.hIcon ? NIM_MODIFY : NIM_ADD;
        icondata.hIcon = icon;
        uFlags = NIF_ICON;
    }
    else
    {
        msg = NIM_DELETE;
    }

    if (tip)
        wcsncpy(icondata.szTip, tip, std::size(icondata.szTip));

    if (*icondata.szTip)
        uFlags |= NIF_TIP;

    if (icondata.uCallbackMessage && icondata.hWnd)
        uFlags |= NIF_MESSAGE;

    icondata.uFlags = uFlags;
    auto const ret = Shell_NotifyIcon(msg, &icondata);

    if (msg == NIM_DELETE || !ret)
        icondata.hIcon = nullptr;

    return ret;
}

LRESULT CTrayIcon::OnTrayNotification(WPARAM id, LPARAM event) const noexcept
{
    if (id != icondata.uID || (event != WM_RBUTTONUP && event != WM_LBUTTONDBLCLK))
        return 0;

    // resource menu with same ID as icon will be used as popup menu
    CMenu menu;
    if (!menu.LoadMenu(icondata.uID))
        return 0;

    CMenu* submenu = menu.GetSubMenu(0);
    if (!submenu)
        return 0;

    if (event == WM_RBUTTONUP)
    {
        ::SetMenuDefaultItem(submenu->m_hMenu, 0, true);
        CPoint mouse;
        ::GetCursorPos(&mouse);
        ::SetForegroundWindow(icondata.hWnd);
        ::TrackPopupMenu(submenu->m_hMenu, 0, mouse.x, mouse.y, 0, icondata.hWnd, nullptr);
    }
    else
    {
        ::SendMessage(icondata.hWnd, WM_COMMAND, submenu->GetMenuItemID(0), 0);
    }

    return 1;
}

bool CTrayIcon::SetIcon(wchar_t const* resname, wchar_t const* tip) noexcept
{
    return SetIcon(resname ? AfxGetApp()->LoadIcon(resname) : nullptr, tip);
}

bool CTrayIcon::DisableTrayIcon() noexcept
{
    return SetIcon(HICON{}, nullptr);
}