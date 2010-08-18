#ifndef SETTINGS_H
#define SETTINGS_H

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

#endif