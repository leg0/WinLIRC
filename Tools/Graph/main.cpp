#include <Windows.h>
#include <tchar.h>
#include "Opengl.h"
#include "irdriver.h"
#include "Settings.h"
#include "guicon.h"
#include "../../DLL/Common/LIRCDefines.h"

#define	IDT_TIMER1 1

//===========================
Opengl			opengl;
CIRDriver		irDriver;
//===========================

hardware const* hw = NULL;

HWND		mainWindowHandle	= NULL;
UINT_PTR	timerID				= NULL;

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
	wcex.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= _T("OpenglGraph");
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	mainWindowHandle = CreateWindowEx(0,_T("OpenglGraph"),_T("IRGraph"),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,800,200,NULL,NULL,hInstance,NULL);

	ShowWindow(mainWindowHandle, nCmdShow);
	UpdateWindow(mainWindowHandle);

	opengl.setupOpengl(mainWindowHandle);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	//====================
	MSG msg;
	Settings settings;
	TCHAR pluginPath[128];
	//====================

	//RedirectIOToConsole();

	SetCurrentDirectory(_T(".\\plugins\\"));

	if(settings.getPluginName(pluginPath)) {

		if(irDriver.loadPlugin(pluginPath)) {

			hw = irDriver.getHardware();

			if(hw==NULL ) {
				MessageBox(NULL,_T("The plugin doesn't export the required functions."),_T("Error"),MB_OK);
				return 0;
			}
		}
		else {
			MessageBox(NULL,_T("Loading plugin failed."),_T("Error"),MB_OK);
			return 0;
		}
	}
	else {
		MessageBox(NULL,_T("No valid plugins found."),_T("Error"),MB_OK);
		return 0;
	}

	if(hw->rec_mode!=LIRC_MODE_MODE2) {
		MessageBox(NULL,_T("The graphing tool is incompatible with this plugin."),_T("Error"),MB_OK);
		return 0;
	}

	if(!irDriver.init()) {
		MessageBox(NULL,_T("Initialising plugin failed."),_T("Error"),MB_OK);
		return 0;
	}

	createWindow(hInstance,nCmdShow);

	SetTimer(mainWindowHandle,IDT_TIMER1,16,&timerFunction);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	irDriver.deinit();

	return (int) msg.wParam;
}