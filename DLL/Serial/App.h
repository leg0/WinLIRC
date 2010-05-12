#pragma once

// App

class App : public CWinApp
{
public:

	App();           // protected constructor used by dynamic creation
	virtual ~App();

	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
};


