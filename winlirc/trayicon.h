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
 */

#pragma once

class CTrayIcon : public CCmdTarget {

protected:
	DECLARE_DYNAMIC(CTrayIcon)
	NOTIFYICONDATA icondata;

public:
	CTrayIcon(UINT uID);
	~CTrayIcon();

	bool SetIcon			(UINT uID);
	bool SetIcon			(HICON hicon, LPCTSTR tip);
	bool SetIcon			(LPCTSTR resname, LPCTSTR tip);
	bool SetStandardIcon	(LPCTSTR iconname, LPCTSTR tip);
	void SetNotificationWnd	(CWnd *notifywnd, UINT message);
	void DisableTrayIcon	();

	LRESULT OnTrayNotification(WPARAM id, LPARAM event);
};
