// AudioCapture.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include "WinlircAudioIn.h"

int _tmain(int argc, _TCHAR* argv[])
{
	loadSetupGui();
	init(0);

	while(true) {
		Sleep(100);
	}

	deinit();

	return 0;
}

