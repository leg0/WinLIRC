#include "web_server.h"

#include <chrono>
#include <filesystem>


namespace {
    std::filesystem::path getDocumentRoot()
    {
        char filename[MAX_PATH + 1];
        size_t const filenameLength = GetModuleFileNameA(nullptr, filename, std::size(filename));
        return std::filesystem::path{ std::string_view{filename, filenameLength} }.replace_filename("ui");
    }

    inline namespace mg_utils {
        inline int has_prefix(mg_str const* uri, mg_str const* prefix) noexcept
        {
            return uri->len > prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
        }

        inline int is_equal(mg_str const* s1, mg_str const* s2) noexcept
        {
            return s1->len == s2->len && memcmp(s1->p, s2->p, s2->len) == 0;
        }

        inline std::string_view sv(mg_str const& s) noexcept
        {
            return std::string_view{ s.p, static_cast<size_t>(s.len) };
        }
    }

    struct HttpMessage : winlirc::IHttpMessage
    {
        explicit HttpMessage(http_message const& hm)
            : hm_{ hm }
        { }

        std::string_view body() const override
        {
            return { hm_.body.p, hm_.body.len };
        }

    private:
        http_message const& hm_;
    };
}

winlirc::WebServer::WebServer(uint16_t port, bool localOnly)
    : port_{ port }
    , s_http_server_opts{}
    , stopping_{ false }
    , documentRoot_{ getDocumentRoot().string() }
{
    mg_mgr_init(&mgr_, this);
    std::string const host = localOnly ? "127.0.0.1:" : ":";
    auto nc = mg_bind(&mgr_, (host + std::to_string(port)).c_str(), ev_handler);
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = documentRoot_.c_str();
}

winlirc::WebServer::~WebServer() noexcept
{
    stopping_ = true;
    if (pollThread_.joinable())
        pollThread_.join();
    mg_mgr_free(&mgr_);
}

void winlirc::WebServer::Start()
{
    pollThread_ = std::thread{ [=]() { PollLoop(); } };
}

void winlirc::WebServer::RegisterEndpoint(std::string_view method, std::string_view regex, RouteHandlerFn fn)
{
    if (method == "GET")
        get_handlers.emplace_back(std::regex{ begin(regex), end(regex) }, std::move(fn));
    else if (method == "PUT")
        put_handlers.emplace_back(std::regex{ begin(regex), end(regex) }, std::move(fn));
}

void winlirc::WebServer::ev_handler(mg_connection* nc, int ev, void* ev_data)
{
    auto hm = static_cast<http_message*>(ev_data);
    static_cast<WebServer*>(nc->mgr->user_data)->EventHandler(nc, ev, static_cast<http_message*>(ev_data));
}

static bool ApiMethodHandler(
    std::vector<std::pair<std::regex, winlirc::WebServer::RouteHandlerFn>> const& handlers,
    mg_connection* nc, http_message* hm)
{
    auto uri = sv(hm->uri);
    std::match_results<std::string_view::const_iterator> m;
    for (auto& [re, handler] : handlers)
    {
        if (std::regex_match(begin(uri), end(uri), m, re))
        {
            auto response = handler(m, std::make_unique<HttpMessage>(*hm));
            if (response.status >= 400)
            {
                mg_http_send_error(nc, response.status, nullptr);
            }
            else
            {
                std::string headers;
                if (response.mimeType.has_value())
                    headers = "Content-type: " + *response.mimeType;
                mg_send_head(nc, response.status, response.body.size(), headers.c_str());
                if (!response.body.empty())
                    mg_send(nc, response.body.data(), response.body.size());
            }
            return true;
        }
    }
    return false;
}

bool winlirc::WebServer::ApiGetHandler(mg_connection* nc, http_message* hm)
{
    return ApiMethodHandler(get_handlers, nc, hm);
}

bool winlirc::WebServer::ApiPutHandler(mg_connection* nc, http_message* hm)
{
    return ApiMethodHandler(put_handlers, nc, hm);
}

bool winlirc::WebServer::ApiPostHandler(mg_connection* nc, http_message* hm)
{
    return false;
}

bool winlirc::WebServer::ApiHandler(mg_connection* nc, http_message* hm)
{
    if (sv(hm->method) == GET)
    {
        return ApiGetHandler(nc, hm);
    }
    else if (sv(hm->method) == PUT)
    {
        return ApiPutHandler(nc, hm);
    }
    else if (sv(hm->method) == POST)
    {
        return ApiPostHandler(nc, hm);
    }
    else
    {
        mg_printf(nc, "%s",
            "HTTP/1.0 501 Not Implemented\r\n"
            "Content-Length: 0\r\n\r\n");
        return true;
    }
}

void winlirc::WebServer::EventHandler(mg_connection* nc, int ev, http_message* hm)
{
    switch (ev)
    {
    case MG_EV_HTTP_REQUEST:
        if (sv(hm->uri).starts_with(api_prefix))
        {
            if (!ApiHandler(nc, hm))
            {
                mg_printf(nc, "%s",
                    "HTTP/1.0 404 Not Found\r\n"
                    "Content-Length: 0\r\n\r\n");
            }
        }
        else
        {
            // Serve static content
            mg_serve_http(nc, hm, s_http_server_opts);
        }
        break;
    default:
        break;
    }
}

void winlirc::WebServer::PollLoop()
{
    std::chrono::milliseconds const t{ 500 };
    while (!stopping_.load())
    {
        mg_mgr_poll(&mgr_, t.count());
    }
}

