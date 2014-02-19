#pragma once

namespace irtiny
{
    class Settings
    {
    public:
        static std::wstring const s_defaultPort;

        void save() const; // to ini file
        void load();

        std::wstring port() const { return port_; }
        void port(std::wstring newPort) { port_ = std::move(newPort); }

    private:
        std::wstring port_;
    };

} // namespace irtiny
