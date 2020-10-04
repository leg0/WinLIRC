#include "stdafx.h"
#include "Settings.h"
#include <string>

namespace irtiny
{
    namespace
    {
        std::wstring getCurrentDirectory()
        {
            wchar_t currentDirectory[MAX_PATH + 1] = { 0 };
            GetCurrentDirectory(MAX_PATH, currentDirectory);
            return currentDirectory;
        }

        std::wstring getIniFilePath()
        {
            return getCurrentDirectory() + L"\\WinLIRC.ini";
        }
    }

    std::wstring const Settings::s_defaultPort = L"\\\\.\\COM1";

    void Settings::load()
    {
        std::wstring const& iniFile = getIniFilePath();
        wchar_t temp[64] = L"";
        GetPrivateProfileString(L"SerialDevice", L"Port", s_defaultPort.c_str(), temp, _countof(temp) - 1, iniFile.c_str());
        port(temp);
    }

    void Settings::save() const
    {
        std::wstring const iniFilePath = getIniFilePath();

        //
        // if our ini files doesn't exist try and create it
        //
        FILE* file = _wfopen(iniFilePath.c_str(), L"r");
        if (file == nullptr)
            file = _wfopen(iniFilePath.c_str(), L"w");

        if (file != nullptr)
            fclose(file);

        WritePrivateProfileStringW(L"SerialDevice", L"Port", port_.c_str(), iniFilePath.c_str());
    }
}
