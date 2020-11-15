#include "stdafx.h"
#include "wl_debug.h"
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void winlirc_debug(const char* file, int line, char const* format, ...)
{

	va_list args;
	va_start(args,format);

	CStringA s;
	s.Format("(%i): ",line);
	CStringA t;
	t.FormatV(format,args);

	auto const hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	static std::mutex debugOutputMutex;
	std::lock_guard g{ debugOutputMutex };
	WriteConsoleA(hStdOut, file, strlen(file), nullptr, nullptr);
	WriteConsoleA(hStdOut, static_cast<char const*>(s), s.GetLength(), nullptr, nullptr);
	WriteConsoleA(hStdOut, static_cast<char const*>(t), t.GetLength(), nullptr, nullptr);
	WriteConsoleA(hStdOut, "\n", 1, nullptr, nullptr);
}
