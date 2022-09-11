#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

// maximum mumber of lines the output console should have

static const WORD MAX_CONSOLE_LINES = 5000;

void RedirectIOToConsole() {

	// allocate a console for this app

	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text

	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

	coninfo.dwSize.Y = MAX_CONSOLE_LINES;

	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	{
		auto const lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		auto const hConHandle = _open_osfhandle(reinterpret_cast<intptr_t>(lStdHandle), _O_TEXT);

		auto const fp = _fdopen(hConHandle, "w");

		*stdout = *fp;

		setvbuf(stdout, NULL, _IONBF, 0);
	}

	// redirect unbuffered STDIN to the console
	{
		auto const lStdHandle = GetStdHandle(STD_INPUT_HANDLE);

		auto const hConHandle = _open_osfhandle(reinterpret_cast<intptr_t>(lStdHandle), _O_TEXT);

		auto const fp = _fdopen(hConHandle, "r");

		*stdin = *fp;

		setvbuf( stdin, NULL, _IONBF, 0 );
	}

	// redirect unbuffered STDERR to the console
	{
		auto const lStdHandle = GetStdHandle(STD_ERROR_HANDLE);

		auto const hConHandle = _open_osfhandle(reinterpret_cast<intptr_t>(lStdHandle), _O_TEXT);

		auto const fp = _fdopen(hConHandle, "w");

		*stderr = *fp;

		setvbuf(stderr, NULL, _IONBF, 0);
	}
	
	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 

	// point to console as well

	std::ios::sync_with_stdio();

}



//End of File
