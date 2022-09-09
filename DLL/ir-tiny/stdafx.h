#pragma once

#define WINVER 0x0600
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning(push)
#pragma warning(disable: 4996) // use of deprecated functions
#include <atlstr.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlddx.h>
#pragma warning(pop)

#include <cstdint>
#include <cstdio>
#include <ctime>
#include <memory>
#include <string>
#include <thread>
#include <utility>
