#pragma once

#include "../DLL/Common/Socket.h"
#include <array>
#include <string>
#include <string_view>
#include <utility>

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
    static UINT ServerThread(void* srv);

    using Socket = winlirc::Socket;
    static void sendData(Socket const& socket, std::string_view s) noexcept;
    void reply(const char* command, int client, bool success, std::string_view data) const;

    Socket m_server;
    std::array<Socket, MAX_CLIENTS> m_clients;

    int			m_tcp_port = 8765;		//tcp port for server
    CWinThread* m_serverThreadHandle = nullptr;
    CEvent		m_serverThreadEvent;
    int			m_winsockStart;
};
