// gen_ir.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "gen_ir.h"
#include "mysock.h"
#include "config.h"
#include <atlbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CGen_irApp

BEGIN_MESSAGE_MAP(CGen_irApp, CWinApp)
	//{{AFX_MSG_MAP(CGen_irApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CGen_irApp theApp;

////////////////////
// Winamp Stuff 
int init(void) { AFX_MANAGE_STATE(AfxGetStaticModuleState()); return theApp.winampInit(); }
void config(void) { AFX_MANAGE_STATE(AfxGetStaticModuleState()); theApp.winampConfig(); }
void quit(void) { AFX_MANAGE_STATE(AfxGetStaticModuleState()); theApp.winampQuit(); }
winampGeneralPurposePlugin gen_ir = {
	DS_VERSION,							// Data structure version
	"WinLIRC Plugin 0.3",				// Description
	init,								// Initialization function
	config,								// Configuration function
	quit								// Deinitialization function
};
extern "C" { 
	__declspec(dllexport) winampGeneralPurposePlugin *winampGetGeneralPurposePlugin(void)
	{ 
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		return &gen_ir;
	}
}
// End Winamp Stuff
///////////////////


CGen_irApp::CGen_irApp()
{
	initialized=false;
	client.app=this;
}

BOOL CGen_irApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
	return TRUE;
}

void CGen_irApp::HandleButton(const char *name, unsigned int repeat)
{
	int max;
	if((max=buttons.GetSize()) != actions.GetSize())
		return;

	// find the button
	for(int i=0;i<max;i++)
		if(stricmp(buttons[i],name)==0)
			break;

	if(i==max)
		return;

	if(repeat!=0 && acts[actions[i]].repeatable==false)
		return;

	for(int j=0;j<acts[actions[i]].count;j++)
		PostMessage(
			gen_ir.hwndParent,
			acts[actions[i]].message,
			acts[actions[i]].wparam,
			acts[actions[i]].lparam);	
}

void CGen_irApp::WriteConfig()
{
	int max;
	if((max=buttons.GetSize()) != actions.GetSize())
		return;
	
	// Delete key and recreate it
	{
		CRegKey tmp;
		if(tmp.Open(HKEY_LOCAL_MACHINE,"Software\\Winamp")!=ERROR_SUCCESS) 
		{
			MessageBox(NULL,"Can't open registry key.","WinLIRC Plugin",MB_OK);
			return;
		}
		tmp.DeleteSubKey("WinLIRC Plugin");
	}
	
	CRegKey key;

	if(key.Create(HKEY_LOCAL_MACHINE,"Software\\Winamp\\WinLIRC Plugin")!=ERROR_SUCCESS)
	{
		MessageBox(NULL,"Can't recreate registry key.","WinLIRC Plugin",MB_OK);
		return;
	}
	
	// fill it
	for(int i=0;i<max;i++)
		key.SetValue(acts[actions[i]].name,buttons[i]);
}

bool CGen_irApp::ReadConfig()
{
	buttons.RemoveAll();
	actions.RemoveAll();
	
	{
		CRegKey tmp;
		tmp.Create(HKEY_LOCAL_MACHINE,"Software\\Winamp");
	}

	CRegKey key;

	if(key.Create(HKEY_LOCAL_MACHINE,"Software\\Winamp\\WinLIRC Plugin")!=ERROR_SUCCESS) 
		return false;
	int count=0;
	char value[256];
	char data[256];
	value[255]=0;
	data[255]=0;
	DWORD type;
	DWORD valuelen=sizeof(value)-1, datalen=sizeof(data)-1;
	while(RegEnumValue(
		key.m_hKey,
		count++,
		value,
		&valuelen,
		NULL,
		&type,
		(unsigned char *)data,
		&datalen)==ERROR_SUCCESS) 
	{
		valuelen=sizeof(value)-1, datalen=sizeof(data)-1;
		if(type==REG_SZ)
		{
			for(int i=0;i<TOTAL_ACTIONS;i++)
			{
				if(stricmp(data,acts[i].name)==0)
				{
					buttons.Add(value);
					actions.Add(i);
					break;
				}
			}
		}
	}
	return true;
}

int CGen_irApp::winampInit(void)
{
	connected=false;
	initialized=false;
	
	if(!ReadConfig()) return 0;
	
	if(client.Create()==0)
	{
		MessageBox(NULL,"Error creating client socket.","WinLIRC",MB_OK);
		return 0;
	}
	initialized=true;
	if(client.Connect("127.0.0.1",8765)==0)
	{
		if(client.GetLastError()!=WSAEWOULDBLOCK)
		{
			initialized=false;
			client.Close();
			//MessageBox(NULL,"Error connecting to server.","WinLIRC",MB_OK);
			return 0;
		}
	}
	/* now we wait for the callback */
	return 0;
}

void CGen_irApp::winampConfig(void)
{
	if(!initialized)
	{
		if(MessageBox(NULL,"This plugin was never properly initialized.\n"
			"Try again?","WinLIRC",MB_YESNO|MB_ICONQUESTION)==IDYES)
			winampInit();
		return;
	}
	if(!connected)
	{
		MessageBox(NULL,"Error: Not connected to server.\n"
			"Try again later.\n","WinLIRC",MB_OK|MB_ICONERROR);
		return;
	}
	
	Cconfig dlg(this);
	dlg.DoModal();
}

void CGen_irApp::winampQuit(void)
{
	if(initialized)
		client.Close();
}
