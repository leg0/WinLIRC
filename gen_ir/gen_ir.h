// gen_ir.h : main header file for the GEN_IR DLL
//

#if !defined(AFX_GEN_IR_H__D55A1691_CC1B_11D2_8C7F_004005637418__INCLUDED_)
#define AFX_GEN_IR_H__D55A1691_CC1B_11D2_8C7F_004005637418__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "mysock.h"
#include "winamp.h"

/*****************/
/* ACTIONS below */
/*****************/

const struct act {
	char *name;
	bool repeatable;
	int count;
	UINT message;
	WPARAM wparam;
	LPARAM lparam;
};

#define TOTAL_ACTIONS 45
const act acts[TOTAL_ACTIONS] = {
	{ "Volume Up",					true,	1,	WM_COMMAND,	WINAMP_VOLUMEUP,			0	},
	{ "Volume Down",				true,	1,	WM_COMMAND,	WINAMP_VOLUMEDOWN,			0	},
	{ "Volume Up 5",				true,	5,	WM_COMMAND,	WINAMP_VOLUMEUP,			0	},
	{ "Volume Down 5",				true,	5,	WM_COMMAND,	WINAMP_VOLUMEDOWN,			0	},
	{ "Play",						false,	1,	WM_COMMAND, WINAMP_PLAY,				0	},
	{ "Stop",						false,	1,	WM_COMMAND,	WINAMP_STOP,				0	},
	{ "Previous Track",				false,	1,	WM_COMMAND,	WINAMP_PREV,				0	},
	{ "Next Track",					false,	1,	WM_COMMAND, WINAMP_NEXT,				0	},
	{ "Fast Forward",				true,	1,	WM_COMMAND, WINAMP_FF,					0	},
	{ "Rewind",      				true,	1,	WM_COMMAND, WINAMP_REW,					0	},
	{ "Pause",						false,	1,	WM_COMMAND,	WINAMP_PAUSE,				0	},
																						
	{ "Fast Forward 5 seconds",		true,	1,	WM_COMMAND,	WINAMP_FF_5S,				0	},
	{ "Rewind 5 seconds",			true,	1,	WM_COMMAND,	WINAMP_REW_5S,				0	},
	{ "Fade out and Stop",			false,	1,	WM_COMMAND,	WINAMP_FADEOUTSTOP,			0	},
	{ "Stop after current Track",	false,	1,	WM_COMMAND,	WINAMP_STOPAFTCURTRK,		0	},
	{ "Start current Vis-PlugIn",	false,	1,	WM_COMMAND,	WINAMP_STARTVISPLUGIN,		0	},
	{ "Close Winamp",				false,	1,	WM_COMMAND,	WINAMP_CLOSEWINAMP,			0	},
	{ "Current track as bookmark",	false,	1,	WM_COMMAND,	WINAMP_ADDCURBOOKMARK,		0	},
	{ "Play Audio-CD",				false,	1,	WM_COMMAND,	WINAMP_PLAYCD,				0	},
	{ "Toggle Playlist-Editor",		false,	1,	WM_COMMAND,	WINAMP_PLAYLIST_TOGGLE,		0	},
	{ "Go to Top of Playlist",		false,	1,	WM_COMMAND,	WINAMP_PLAYLISTTOP,			0	},
	{ "Go to End of Playlist",		false,	1,	WM_COMMAND,	WINAMP_PLAYLISTEND,			0	},
	{ "Move back 10 tracks",		false,	1,	WM_COMMAND,	WINAMP_PREV10,				0	},
																						
	{ "Open URL Dialog",			false,	1,	WM_COMMAND,	WINAMP_DLG_OPENURL,			0	},
	{ "Load File Dialog",			false,	1,	WM_COMMAND,	WINAMP_DLG_OPENFILE,		0	},
	{ "Toggle Preferences Dialog",	false,	1,	WM_COMMAND,	WINAMP_DLG_PREFS_TOGGLE,	0	},
	{ "Open File Info Dialog",		false,	1,	WM_COMMAND,	WINAMP_DLG_FILEINFO,		0	},
	{ "Open Visualization options", false,	1,	WM_COMMAND,	WINAMP_DLG_VISOPTIONS,		0	},
	{ "Open Vis-PlugIn options",	false,	1,	WM_COMMAND,	WINAMP_DLG_VISPLUGIN,		0	},
	{ "Toggle About Box",			false,	1,	WM_COMMAND,	WINAMP_DLG_ABOUT_TOGGLE,	0	},
	{ "Open 'Jump to time'-Dialog", false,	1,	WM_COMMAND,	WINAMP_DLG_JUMPTOTIME,		0	},
	{ "Open 'Jump to file'-Dialog", false,	1,	WM_COMMAND,	WINAMP_DLG_JUMPTOFILE,		0	},
	{ "Open 'Select Skin'-Dialog",	false,	1,	WM_COMMAND,	WINAMP_DLG_SELECTSKIN,		0	},
	{ "Conigure current Vis-PlugIn",false,	1,	WM_COMMAND,	WINAMP_DLG_VISPLUGINCFG,	0	},
	{ "Load an Equalizer-Preset",	false,	1,	WM_COMMAND,	WINAMP_DLG_EQLOADPRESET,	0	},
	{ "Save Equalizer as",			false,	1,	WM_COMMAND,	WINAMP_DLG_EQSAVEAS,		0	},

	{ "Toggle always on top",		false,	1,	WM_COMMAND,	WINAMP_AOT_TOGGLE,			0	},
	{ "Display: Elapsed Time",		false,	1,	WM_COMMAND,	WINAMP_DISPLAYELAPSED,		0	},
	{ "Display: Remaining Time",	false,	1,	WM_COMMAND,	WINAMP_DISPLAYREMAIN,		0	},
	{ "Toggle DoubleSize-Mode",		false,	1,	WM_COMMAND,	WINAMP_DOUBLESIZE_TOGGLE,	0	},
	{ "Toggle Equalizer",			false,	1,	WM_COMMAND,	WINAMP_EQ_TOGGLE,			0	},
	{ "Toggle main window visible", false,	1,	WM_COMMAND,	WINAMP_VISIBLE_TOGGLE,		0	},
	{ "Toggle Minibrowser",			false,	1,	WM_COMMAND,	WINAMP_BROWSER_TOGGLE,		0	},
	{ "Toggle Repeat Mode",			false,	1,	WM_COMMAND,	WINAMP_REPEAT_TOGGLE,		0	},
	{ "Toggle Shuffle Mode",		false,	1,	WM_COMMAND,	WINAMP_SHUFFLE_TOGGLE,		0	},

};

/*****************/
/* ACTIONS above */
/*****************/





typedef struct {
	int version;
	char *description;
	int (*init)();
	void (*config)();
	void (*quit)();
	HWND hwndParent;
	HINSTANCE hDllInstance;
} winampGeneralPurposePlugin;

/* Data structure version */
#define DS_VERSION 0x10

/* Winamp functions and arrays */
extern winampGeneralPurposePlugin *gen_plugins[256];
typedef winampGeneralPurposePlugin * (*winampGeneralPurposePluginGetter)();


/////////////////////////////////////////////////////////////////////////////
// CGen_irApp
// See gen_ir.cpp for the implementation of this class
//

class CGen_irApp : public CWinApp
{
public:
	CGen_irApp();

	int winampInit(void);
	void winampConfig(void);
	void winampQuit(void);

	bool ReadConfig(void);
	void WriteConfig(void);

	void HandleButton(const char *name, unsigned int repeat);

	bool initialized;
	bool connected;

	CStringArray buttons;
	CDWordArray actions;

private:

	CMySocket client;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGen_irApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CGen_irApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEN_IR_H__D55A1691_CC1B_11D2_8C7F_004005637418__INCLUDED_)
