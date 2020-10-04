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

#include "stdafx.h"
#include "server.h"
#include "remote.h"
#include <atlbase.h> //password stuff
#include "winlirc.h"
#include "drvdlg.h"

#include <algorithm>
#include <string>
#include <string_view>

using namespace std::string_literals;
using namespace std::string_view_literals;

constexpr int LISTENQ = 4;        // listen queue size
constexpr size_t MAX_DATA = 1024; // longest message a client can send

UINT Cserver::ServerThread(void* srv)
{
    static_cast<Cserver*>(srv)->ThreadProc();
    return 0;
}

Cserver::Cserver()
{
    WSADATA data;
    m_winsockStart = WSAStartup(MAKEWORD(2, 0), &data);
}

Cserver::~Cserver()
{
    stopServer();
    WSACleanup();
}

bool Cserver::restartServer()
{
    stopServer();
    return startServer();
}

bool Cserver::startServer()
{

    if (m_winsockStart != 0)
    {
        MessageBoxW(nullptr,
            L"Could not initialize Windows Sockets.\n"
            L"Note that this program requires WinSock 2.0 or higher.",
            L"WinLIRC", MB_OK);
        return false;
    }

    Socket server{ socket(AF_INET, SOCK_STREAM, 0) };

    if (!server)
    {
        WL_DEBUG("socket failed, WSAGetLastError=%d\n", WSAGetLastError());
        return false;
    }
    sockaddr_in serv_addr = { 0 };
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(config.serverPort);

    if (config.localConnectionsOnly)
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server.get(), reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) == SOCKET_ERROR)
    {
        WL_DEBUG("bind failed\n");
        return false;
    }

    if (listen(server.get(), LISTENQ) == SOCKET_ERROR)
    {
        WL_DEBUG("listen failed\n");
        return false;
    }

    m_server = std::move(server);

    // THREAD_PRIORITY_IDLE combined with the HIGH_PRIORITY_CLASS
    // of this program still results in a really high priority. (16 out of 31)
    m_serverThreadHandle = AfxBeginThread(ServerThread, this, THREAD_PRIORITY_IDLE);
    if (m_serverThreadHandle == nullptr) {
        WL_DEBUG("AfxBeginThread failed\n");
        return false;
    }
    return true;
}

void Cserver::stopServer()
{
    KillThread(&m_serverThreadHandle, &m_serverThreadEvent);

    for (auto& client : m_clients)
    {
        client.reset();
    }

    m_server.reset();
}

void Cserver::sendToClients(std::string_view s) const
{
    for (auto& client : m_clients)
    {
        sendData(client, s);
    }
}

void Cserver::sendData(Socket const& socket, std::string_view s) noexcept
{
    if (!socket || s.empty())
        return;

    while (!s.empty())
    {
        int const sent = send(socket.get(), s.data(), s.size(), 0);
        if (sent == SOCKET_ERROR)
            break;
        s.remove_prefix(sent);
    }
}

void Cserver::reply(const char* command, int client, bool success, std::string_view data) const
{
    std::string packet = "BEGIN\n"s;
    packet += command;
    packet += success ? "\nSUCCESS\n" : "\nERROR\n";
    packet += data;
    packet += "END\n";
    sendData(m_clients[client], packet);
}

void Cserver::ThreadProc()
{
    if (!m_server)
        return;

    //=========================================
    int		i;
    CEvent	serverEvent;
    CEvent	clientEvent[MAX_CLIENTS];
    char	clientData[MAX_CLIENTS][MAX_DATA];
    char	toparse[MAX_DATA];
    HANDLE	events[MAX_CLIENTS + 2];
    //=========================================

    WSAEventSelect(m_server.get(), serverEvent, FD_ACCEPT);

    for (i = 0; i < MAX_CLIENTS; i++)
    {
        clientData[i][0] = '\0';
    }

    for (;;)
    {
        int count = 0;
        events[count++] = m_serverThreadEvent;	// exit event
        events[count++] = serverEvent;

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (m_clients[i]) {
                events[count++] = clientEvent[i];
            }
        }

        DWORD const res = WaitForMultipleObjects(count, events, FALSE, INFINITE);

        if (res == WAIT_OBJECT_0)
        {
            WL_DEBUG("ServerThread terminating\n");
            return;
        }
        else if (res == (WAIT_OBJECT_0 + 1))
        {
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (!m_clients[i]) break;
            }

            Socket tempSocket{ accept(m_server.get(), nullptr, nullptr) };
            if (i == MAX_CLIENTS)
            {
                auto errorMsg = "Sorry the server is full.\n"sv;
                ::send(tempSocket.get(), errorMsg.data(), errorMsg.size(), 0);
                continue;
            }

            if (!tempSocket)
            {
                WL_DEBUG("accept() failed\n");
                continue;
            }
            m_clients[i] = std::move(tempSocket);

            WSAEventSelect(m_clients[i].get(), clientEvent[i], FD_CLOSE | FD_READ);
            clientEvent[i].ResetEvent();
            clientData[i][0] = '\0';
            WL_DEBUG("Client connection %d accepted\n", i);
        }
        else /* client closed or data received */
        {
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                if (m_clients[i])
                {
                    if (res == (WAIT_OBJECT_0 + (2 + i)))
                    {
                        /* either we got data or the connection closed */
                        int curlen = (int)strlen(clientData[i]);
                        int maxlen = MAX_DATA - curlen - 1;
                        int bytes = recv(m_clients[i].get(), clientData[i] + curlen, maxlen, 0);

                        if (bytes == 0 || bytes == SOCKET_ERROR)
                        {
                            /* Connection was closed or something's screwy */
                            m_clients[i].reset();
                            WL_DEBUG("Client connection %d closed\n", i);
                        }
                        else /* bytes > 0, we read data */
                        {
                            clientData[i][curlen + bytes] = '\0';
                            char* cur = clientData[i];
                            for (;;) {
                                char* nl = strchr(cur, '\n');
                                if (nl == nullptr) {
                                    if (cur != clientData[i])
                                        memmove(clientData[i], cur, strlen(cur) + 1);
                                    break;
                                }
                                else {
                                    *nl = '\0';
                                    // ----------------------------
                                    // Do something with the received line (cur)
                                    WL_DEBUG("Got string: %s\n", cur);

                                    strcpy_s(toparse, cur);
                                    char* command = strtok(toparse, " \t\r");	// strtok is not thread safe ..

                                    if (!command) //ignore lines with only whitespace
                                    {
                                        cur = nl + 1;
                                        break;
                                    }

                                    std::pair<bool, std::string> response;
                                    if (_stricmp(command, "VERSION") == 0)
                                        response = parseVersion(cur);
                                    else if (_stricmp(command, "LIST") == 0)
                                        response = parseListString(cur);
                                    else if (_stricmp(command, "SET_TRANSMITTERS") == 0)
                                        response = parseTransmitters(cur);
                                    else if (_stricmp(command, "SEND_ONCE") == 0)
                                        response = parseSendString(cur);
                                    else
                                        response.first = false;

                                    reply(cur, i, response.first, response.second);
                                    cur = nl + 1;
                                }
                            }

                        }

                        break;
                    }
                }
            }
        }
    }
}

std::pair<bool, std::string> Cserver::parseSendString(char const* string)
{
    CSingleLock lock(&CS_global_remotes, TRUE);

    char remoteName[128] = "";
    char keyName[128] = "";
    int repeats = 0;
    int result = sscanf_s(string, "%*s %s %s %i",
        remoteName, static_cast<uint32_t>(sizeof(remoteName)),
        keyName, static_cast<uint32_t>(sizeof(keyName)),
        &repeats);

    if (result < 2)
        return { false, "DATA\n1\nremote or code missing\n"s };

    ir_remote* const sender = get_remote_by_name(global_remotes, remoteName);

    if (sender == nullptr)
        return { false, "DATA\n1\nremote not found\n"s };

    ir_ncode* const codes = get_code_by_name(sender->codes, keyName);

    if (codes->name == nullptr)
        return { false, "DATA\n1\ncode not found\n"s };

    repeats = max(repeats, sender->min_repeat);
    repeats = min(repeats, 10);	// sanity check

    // reset toggle masks

    if (has_toggle_mask(sender)) {
        sender->toggle_mask_state = 0;
    }

    if (has_toggle_bit_mask(sender)) {
        sender->toggle_bit_mask_state = (sender->toggle_bit_mask_state ^ sender->toggle_bit_mask);
    }

    BOOL const success = app.dlg->driver.sendIR(sender, codes, repeats);

    if (!success)
        return { false, "DATA\n1\nsend failed/not supported\n"s };

    app.dlg->GoBlue();

    return { true, ""s };
}

std::pair<bool, std::string> Cserver::parseListString(const char* string)
{
    //====================
    char* remoteName;
    char* codeName;
    int		n;
    struct ir_remote* all;
    //====================

    CSingleLock lock(&CS_global_remotes, TRUE);

    remoteName = strtok(nullptr, " \t\r");
    codeName = strtok(nullptr, " \t\r");
    n = 0;
    all = global_remotes;

    if (!remoteName)
    {
        while (all)
        {
            n++;
            all = all->next;
        }

        std::string response;
        if (n != 0)
        {
            response = "DATA\n"s + std::to_string(n) + "\n";
            all = global_remotes;

            while (all)
            {
                response += all->name;
                response += "\n";
                all = all->next;
            }
        }

        return { true, std::move(response) };
    }

    // find remote name 

    if (remoteName) {

        all = get_remote_by_name(all, remoteName);
        if (!all)
            return { false, "DATA\n1\nunknown remote: "s + remoteName + "\n" };
    }

    if (remoteName && !codeName)
    {
        ir_ncode* allcodes = all->codes;
        while (allcodes->name)
        {
            n++;
            allcodes++;
        }
        std::string response;
        if (n != 0)
        {
            response = "DATA\n"s + std::to_string(n) + "\n";
            allcodes = all->codes;

            while (allcodes->name)
            {
                response += allcodes->name;
                response += "\n";
                allcodes++;
            }
        }
        return { true, std::move(response) };
    }

    if (remoteName && codeName)
    {
        ir_ncode* const code = get_code_by_name(all->codes, codeName);
        if (code)
        {
            char buf[100];
            sprintf(buf, "DATA\n1\n%016llX %s\n", code->code, code->name);
            return { true, buf };
        }
        else
        {
            return { false, "DATA\n1\nunknown code: "s + codeName + "\n" };
        }
    }
    return { false, "DATA\n1\nerror\n"s };
}

std::pair<bool, std::string> Cserver::parseVersion(const char* string)
{
    if (strtok(nullptr, " \t\r") == nullptr)
        return { true, "DATA\n1\n"s + id + "\n" };
    else
        return { false, "DATA\n1\nbad send packet\n"s };
}

std::pair<bool, std::string> Cserver::parseTransmitters(const char* string)
{
    DWORD transmitterMask = 0;

    while (char* transmitterNumber = strtok(nullptr, " \t\r"))
    {
        int const transNumber = atoi(transmitterNumber);
        if (transNumber == 0)
            continue;

        if (transNumber > 32)
            return { false, "DATA\n1\ncannot support more than 32 transmitters\n"s };

        transmitterMask |= 1 << (transNumber - 1);
    }

    if (!app.dlg->driver.setTransmitters(transmitterMask))
        return { false, "DATA\n1\nSetTransmitters failed/not supported\n"s };
    else
        return { true, ""s };
}

