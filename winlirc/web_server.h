#pragma once

#include <mongoose.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace winlirc {

    using svmatch = std::match_results<std::string_view::const_iterator>;

    struct IHttpMessage
    {
        IHttpMessage() = default;
        IHttpMessage(IHttpMessage const&) = delete;
        IHttpMessage(IHttpMessage&&) = delete;
        IHttpMessage& operator=(IHttpMessage const&) = delete;
        IHttpMessage& operator=(IHttpMessage&&) = delete;
        virtual ~IHttpMessage() = default;

        virtual std::string_view body() const = 0;
    };

    using IHttpMessagePtr = std::unique_ptr<IHttpMessage const>;

    struct WebServer
    {
        struct response
        {
            uint32_t status = 200;
            std::optional<std::string> mimeType;
            std::string body;
        };

        explicit WebServer(uint16_t port, bool localOnly = true);
        WebServer(WebServer const&) = delete;
        WebServer(WebServer&&) = delete;
        WebServer& operator=(WebServer const&) = delete;
        WebServer& operator=(WebServer&&) = delete;

        ~WebServer() noexcept;

        void Start();

        using RouteHandlerFn = std::function<response(svmatch const&, std::unique_ptr<IHttpMessage const>)>;
        void RegisterEndpoint(std::string_view method, std::string_view regex, RouteHandlerFn fn);

    private:
        static void ev_handler(mg_connection* nc, int ev, void* ev_data);

        bool ApiGetHandler(mg_connection* nc, http_message* hm);
        bool ApiPutHandler(mg_connection* nc, http_message* hm);
        bool ApiPostHandler(mg_connection* nc, http_message* hm);
        bool ApiHandler(mg_connection* nc, http_message* hm);
        void EventHandler(mg_connection* nc, int ev, http_message* hm);
        void PollLoop();

    private:
        mg_mgr mgr_;
        uint16_t const port_;
        mg_serve_http_opts s_http_server_opts;
        std::string_view const api_prefix = "/api/v1";
        std::string_view const GET = "GET";
        std::string_view const PUT = "PUT";
        std::string_view const POST = "POST";
        std::thread pollThread_;
        std::atomic<bool> stopping_;
        std::string const documentRoot_;

        std::vector<std::pair<std::regex, RouteHandlerFn>> get_handlers;
        std::vector<std::pair<std::regex, RouteHandlerFn>> put_handlers;
        std::vector<std::pair<std::regex, RouteHandlerFn>> post_handlers;
    };
}
