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
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 */

#pragma once

#include <winlirc-common/Event.h>
#include <winlirc-common/Socket.h>

#include <array>
#include <string_view>
#include <thread>

using winlirc::Socket;

static constexpr size_t MAX_CLIENTS = 8;

class Cserver
{
public:

    Cserver();
    ~Cserver();

    bool startServer();
    void stopServer();
    bool restartServer();
    void sendToClients(std::string_view s) const;
    std::pair<bool, std::string> parseSendString(char const* string);
    std::pair<bool, std::string> parseListString(const char* string);
    std::pair<bool, std::string> parseVersion(const char* string);
    std::pair<bool, std::string> parseTransmitters(const char* string);

private:
    void ThreadProc();

    static void sendData(Socket const& socket, std::string_view s) noexcept;
    void reply(const char* command, int client, bool success, std::string_view data) const;

    Socket m_server;
    std::array<Socket, MAX_CLIENTS> m_clients;

    int			m_tcp_port = 8765;		//tcp port for server
    std::thread m_serverThreadHandle;
    winlirc::Event m_serverThreadEvent;
    int			m_winsockStart;
};
