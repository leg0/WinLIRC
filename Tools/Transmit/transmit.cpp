#include <windows.h>
#include <stdio.h>

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	LRESULT copyDataResult;
	HWND pOtherWnd = FindWindowA(NULL, "WinLirc");
	if (pOtherWnd)
	{
		COPYDATASTRUCT cpd;
		cpd.dwData = 0;
		cpd.cbData = strlen(lpCmdLine)+1;
		cpd.lpData = (void*)lpCmdLine;
		copyDataResult = SendMessageA(pOtherWnd,WM_COPYDATA,(WPARAM)hInstance,(LPARAM)&cpd);
        // copyDataResult has value returned by other app
	}
	else
	{
		return 1;
	}
	return 0;
}


