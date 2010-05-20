#ifndef SETTINGS_H
#define SETTINGS_H

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

#endif

