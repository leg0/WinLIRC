// App.cpp : implementation file
//

#include "stdafx.h"
#include "App.h"
#include "SerialDialog.h"


// App

//IMPLEMENT_DYNCREATE(App, CWinApp)

App::App()
{
}

App::~App()
{
}

BOOL App::InitInstance()
{
	// TODO:  perform and per-thread initialization here


	return TRUE;
}

int App::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinApp::ExitInstance();
}

BEGIN_MESSAGE_MAP(App, CWinApp)
END_MESSAGE_MAP()


// App message handlers
