#ifndef SETTINGS_H
#define SETTINGS_H

#include <tchar.h>

class Settings 
{

public:
	Settings();

	void	setAudioDeviceName(TCHAR *name);
	void	getAudioDeviceName(TCHAR *out);

	void	setAudioFormat(int format);
	int		getAudioFormat();

	void	setChannel(bool left);
	bool	getChannel();			//left is true

	void	saveSettings();			// to ini file
	void	loadSettings();

private:

	//========================
	TCHAR	deviceName[32];			// 32 is max length
	int		audioFormat;
	bool	leftChannel;			// if mono ignore
	//========================
};

#endif

