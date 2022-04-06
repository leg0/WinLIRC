#include "stdafx.h"
#include "winlirc/winlirc_api.h"
#include "winlircapp.h"
#include <algorithm>

WINLIRC_API int winlirc_settings_get_int(
	_In_z_ wchar_t const* section,
	_In_z_ wchar_t const* setting,
	int defaultValue)
{
	return app.config->readInt(section, setting, defaultValue);
}

WINLIRC_API size_t winlirc_settings_get_wstring(
	_In_z_ wchar_t const* section,
	_In_z_ wchar_t const* setting,
	_Out_writes_(outSize)  wchar_t* out, size_t outSize,
	_In_opt_z_ wchar_t const* defaultValue)
{
	return app.config->readString(section, setting, out, outSize, defaultValue);
}

WINLIRC_API void winlirc_settings_set_int(
	_In_z_ wchar_t const* section,
	_In_z_ wchar_t const* setting,
	int value)
{
	app.config->writeInt(section, setting, value);
}

WINLIRC_API void winlirc_settings_set_wstring(
	_In_z_ wchar_t const* section,
	_In_z_ wchar_t const* setting,
	_In_z_ wchar_t const* str)
{
	app.config->writeString(section, setting, str);
}
