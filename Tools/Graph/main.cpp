#include <Windows.h>
#include <tchar.h>
#include "Opengl.h"
#include "irdriver.h"
#include "guicon.h"
#include <iterator>
#include <filesystem>
#include <optional>

#define	IDT_TIMER1 1

using namespace std::literals;

static std::optional<std::filesystem::path> getPluginPath(std::filesystem::path const& dir)
{
    auto path = dir / "WinLIRC.ini";
    wchar_t pluginName[128];
    auto count = GetPrivateProfileString(L"WinLIRC", L"Plugin", nullptr, pluginName, std::size(pluginName), path.c_str());

    if (count)
        return pluginName;

    return std::nullopt;
}

//===========================
Opengl			opengl;
CIRDriver		irDriver;
//===========================

hardware const* hw = nullptr;

HWND		mainWindowHandle = nullptr;
UINT_PTR	timerID = 0;

void CALLBACK timerFunction(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    if (idEvent == IDT_TIMER1)
    {
        while (hw->data_ready())
        {
            opengl.pushValue(hw->readdata(0));
        }
        opengl.drawGraph();
    }
}

LRESULT CALLBACK WndProc(
    HWND	hWnd,			// Handle For This Window
    UINT	uMsg,			// Message For This Window
    WPARAM	wParam,			// Additional Message Information
    LPARAM	lParam)			// Additional Message Information
{
    switch (uMsg)									// Check For Windows Messages
    {
    case WM_SYSCOMMAND:							// Intercept System Commands
        break;									// Exit

    case WM_PAINT:
        opengl.redrawScene();
        break;

    case WM_CLOSE:								// Did We Receive A Close Message?
        PostQuitMessage(0);						// Send A Quit Message
        return 0;

    case WM_SIZE:								// Resize The OpenGL Window
        opengl.setViewport(LOWORD(lParam), HIWORD(lParam));
        break;

    }

    // Pass All Unhandled Messages To DefWindowProc
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void createWindow(HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASSEX	wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIconW(nullptr, IDI_WINLOGO);
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"OpenglGraph";
    wcex.hIconSm = nullptr;

    RegisterClassExW(&wcex);

    mainWindowHandle = CreateWindowExW(0, L"OpenglGraph", L"IRGraph",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 200,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(mainWindowHandle, nCmdShow);
    UpdateWindow(mainWindowHandle);

    opengl.setupOpengl(mainWindowHandle);
}

int wmain(int argc, wchar_t** argv);

int APIENTRY wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow)
{
    int argc;
    wchar_t** argv = CommandLineToArgvW(lpCmdLine, &argc);

    return wmain(argc, argv);
}

int wmain(int argc, wchar_t** argv)
{
    std::filesystem::path const pluginsDir = (argc == 2)
        ? argv[1]
        : std::filesystem::current_path() / "plugins";

    if (!std::filesystem::is_directory(pluginsDir))
    {
        MessageBoxW(nullptr, L"Plugins directory not found.", pluginsDir.c_str(), MB_ICONERROR);
        return 0;
    }

    auto pluginName = (argc == 3) ? std::optional{argv[2]} : std::nullopt;
    if (argc > 3)
    {
        MessageBoxW(nullptr, L"Usage: IRGraph.exe <path to plugins> [plugin name]", L"Error", MB_ICONERROR);
        return 0;
    }

    auto pp = pluginName
        ? pluginsDir / *pluginName
        : getPluginPath(pluginsDir);
    if (pp)
    {
        if (irDriver.loadPlugin(*pp))
        {
            hw = irDriver.getHardware();
            if (hw == nullptr) {
                MessageBoxW(nullptr, L"The plugin doesn't export the required functions.", L"Error", MB_ICONERROR);
                return 0;
            }
        }
        else
        {
            auto error = L"Loading plugin failed.\r\n\r\n"s + pp->c_str();
            MessageBoxW(nullptr, error.c_str(), L"Error", MB_ICONERROR);
            return 0;
        }
    }


    if (hw->rec_mode != LIRC_MODE_MODE2)
    {
        MessageBoxW(nullptr, L"The graphing tool is incompatible with this plugin.", L"Error", MB_ICONERROR);
        return 0;
    }

    if (!irDriver.init())
    {
        MessageBoxW(nullptr, L"Initialising plugin failed.", L"Error", MB_ICONERROR);
        return 0;
    }

    createWindow(nullptr, SW_SHOW);

    SetTimer(mainWindowHandle, IDT_TIMER1, 16, &timerFunction);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    irDriver.deinit();

    return (int)msg.wParam;
}
