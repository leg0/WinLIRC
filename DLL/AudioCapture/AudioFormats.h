#ifndef AUDIOFORMATS_H
#define AUDIOFORMATS_H

#include <Windows.h>
#include <tchar.h>

namespace AudioFormats {

	bool	formatSupported	(int format);
	void	getFormatString	(int format, TCHAR *outString, int noBuffElements);
	void	getFormatDetails(int format, BOOL *outStereo, int *outFrequency);
	int		getAudioIndex	(TCHAR *audioDeviceName);
}

#endif
