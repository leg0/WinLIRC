#ifndef SETTINGS_H
#define SETTINGS_H

#include "stdafx.h"
#include <tchar.h>

class Settings 
{

public:
	Settings();

	void	saveSettings();			// to ini file
	void	loadSettings();

	//========================
	CString	port;
	CString	speed;
	BOOL	animax;
	int		deviceType;
	int		transmitterType;
	int		virtualPulse;
	int		sense;
	//========================

private:

};

#endif

