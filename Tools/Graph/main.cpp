#include <Windows.h>
#include <tchar.h>
#include "Opengl.h"
#include <winlirc-plugin/Plugin.h>
#include "Settings.h"
#include "guicon.h"
#include <winlirc-common/Event.h>

#define	IDT_TIMER1 1

//===========================
Opengl			opengl;
Plugin irDriver;
//===========================

hardware const* hw = nullptr;

HWND		mainWindowHandle	= nullptr;
UINT_PTR	timerID				= 0;

VOID CALLBACK timerFunction(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {

	if(idEvent==IDT_TIMER1) {

		while(hw->data_ready()) {
			opengl.pushValue(hw->readdata(0));
		}
		opengl.drawGraph();
	}
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			break;									// Exit
		}
		case WM_PAINT:
			{
				opengl.redrawScene();
				break;
			}
		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;
		}


		case WM_SIZE:								// Resize The OpenGL Window
		{
			//printf("size called %i %i\n",LOWORD(lParam),HIWORD(lParam));
			opengl.setViewport(LOWORD(lParam),HIWORD(lParam));
			break;
		}

	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void createWindow(HINSTANCE hInstance, int nCmdShow) {

	//===============
	WNDCLASSEX	wcex;
	//===============

	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(nullptr, IDI_WINLOGO);
	wcex.hCursor		= LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground	= nullptr;
	wcex.lpszMenuName	= nullptr;
	wcex.lpszClassName	= _T("OpenglGraph");
	wcex.hIconSm		= nullptr;

	RegisterClassEx(&wcex);

	mainWindowHandle = CreateWindowEx(0,_T("OpenglGraph"),_T("IRGraph"),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,800,200,nullptr,nullptr,hInstance,nullptr);

	ShowWindow(mainWindowHandle, nCmdShow);
	UpdateWindow(mainWindowHandle);

	opengl.setupOpengl(mainWindowHandle);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	//RedirectIOToConsole();
	std::filesystem::current_path(L".\\plugins\\");

	if(auto pluginPath = Settings::getPluginName()) {

		if (irDriver = Plugin{ *pluginPath }) {

			hw = irDriver.interface_.getHardware();

			if(hw==nullptr ) {
				MessageBox(nullptr,_T("The plugin doesn't export the required functions."),_T("Error"),MB_OK);
				return 0;
			}
		}
		else {
			MessageBox(nullptr,_T("Loading plugin failed."),_T("Error"),MB_OK);
			return 0;
		}
	}
	else {
		MessageBox(nullptr,_T("No valid plugins found."),_T("Error"),MB_OK);
		return 0;
	}

	if(hw->rec_mode!=LIRC_MODE_MODE2) {
		MessageBox(nullptr,_T("The graphing tool is incompatible with this plugin."),_T("Error"),MB_OK);
		return 0;
	}

	auto e = winlirc::Event::manualResetEvent();
	if(!irDriver.interface_.init(reinterpret_cast<WLEventHandle>(e.get()))) {
		MessageBox(nullptr,_T("Initialising plugin failed."),_T("Error"),MB_OK);
		return 0;
	}

	createWindow(hInstance,nCmdShow);

	SetTimer(mainWindowHandle,IDT_TIMER1,16,&timerFunction);

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	e.setEvent();
	irDriver.interface_.deinit();
	::Sleep(500);

	return (int) msg.wParam;
}