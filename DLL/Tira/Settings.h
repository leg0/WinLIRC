#pragma once

class Settings 
{

public:
	Settings();

	int		getComPort	();
	void	setComPort	(int port);

	void	saveSettings();			// to ini file
	void	loadSettings();

private:

	//==========
	int comPort;
	//==========
};
