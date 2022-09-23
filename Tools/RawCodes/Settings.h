#pragma once

#include <Tchar.h>

class Settings {

public:
	Settings();

	bool getPluginName(TCHAR out[128]);

private:

	//====================
	TCHAR pluginName[128];
	//====================

};
