#pragma once

#include "server.h"
#include "drvdlg.h"
#include "resource.h"
#include "web_server.h"

#include <memory>

class Cwinlirc : public CWinApp
{
public:
	std::unique_ptr<Cdrvdlg> dlg;
	Cserver server;
	std::unique_ptr<winlirc::WebServer> webServer;

	virtual BOOL InitInstance() override;
	virtual int ExitInstance() override;
};

extern Cwinlirc app;

void KillThread(CWinThread **ThreadHandle, CEvent *ThreadEvent);
void KillThread2(CWinThread **ThreadHandle, HANDLE ThreadEvent);
