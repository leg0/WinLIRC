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

#define TOTAL_ACTIONS 11
const act acts[TOTAL_ACTIONS] = {
	{ "Volume Up",		true,	1,	WM_COMMAND,	WINAMP_VOLUMEUP,		0	},
	{ "Volume Down",	true,	1,	WM_COMMAND,	WINAMP_VOLUMEDOWN,		0	},
	{ "Volume Up 5",	true,	5,	WM_COMMAND,	WINAMP_VOLUMEUP,		0	},
	{ "Volume Down 5",	true,	5,	WM_COMMAND,	WINAMP_VOLUMEDOWN,		0	},
	{ "Play",			false,	1,	WM_COMMAND, WINAMP_BUTTON2,			0	},
	{ "Stop",			false,	1,	WM_COMMAND,	WINAMP_BUTTON4,			0	},
	{ "Previous Track",	false,	1,	WM_COMMAND,	WINAMP_BUTTON1,			0	},
	{ "Next Track",		false,	1,	WM_COMMAND, WINAMP_BUTTON5,			0	},
	{ "Fast Forward",	true,	1,	WM_COMMAND, WINAMP_BUTTON5_SHIFT,	0	},
	{ "Rewind",      	true,	1,	WM_COMMAND, WINAMP_BUTTON1_SHIFT,	0	},
	{ "Pause",			false,	1,	WM_COMMAND,	WINAMP_BUTTON3,			0	},
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
