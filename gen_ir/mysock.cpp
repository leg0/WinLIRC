// mysock.cpp : implementation file
//

#include "stdafx.h"
#include "gen_ir.h"
#include "mysock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMySocket

CMySocket::CMySocket()
{	
	curr=0;
}

CMySocket::~CMySocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CMySocket, CAsyncSocket)
	//{{AFX_MSG_MAP(CMySocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CMySocket member functions

void CMySocket::OnConnect(int nErrorCode) 
{
	CAsyncSocket::OnConnect(nErrorCode);
	
	if(nErrorCode!=0)
	{
		app->initialized=false;
		MessageBox(NULL,"Failed to connect to WinLIRC server.","WinLIRC",MB_OK);
		Close();
	}
	else
	{
		app->connected=true;
	}
}

void CMySocket::OnReceive(int nErrorCode) 
{
	CAsyncSocket::OnReceive(nErrorCode);

	if(nErrorCode!=0)
	{
		app->initialized=app->connected=false;
		MessageBox(NULL,"Network connection failed.","WinLIRC",MB_OK);
		Close();
	}
	else
	{	
		if(curr>=MAXLEN) // too much data, lose it
			curr=0;
		int res;
		res=Receive(buf+curr,MAXLEN-curr);
		if(res==SOCKET_ERROR || res==0) return;
		curr+=res;
		// find a newline
		for(int i=0;i<curr;i++)
			if(buf[i]=='\n') break;
		if(i==curr) // no newline
			return;
		int loc=i;
		// found a newline: copy that much of buf into tmp
		char tmp[MAXLEN+1];
		for(i=0;i<=loc;i++)
			tmp[i]=buf[i];
		// null terminate it
		tmp[i]=0;
		// move buf back
		for(i=loc;i<curr;i++)
			buf[i-loc]=buf[i];

		unsigned __int64 keycode;
		unsigned int repeat;
		char keyname[MAXLEN];
		char remotename[MAXLEN];
		if(sscanf(tmp,"%I64x %x %s %[^\n]",&keycode,&repeat,keyname,remotename)!=4)
			return;

		// send it to the app
		app->HandleButton(keyname, repeat);
	}
}

void CMySocket::OnClose(int nErrorCode) 
{
	CAsyncSocket::OnClose(nErrorCode);

	app->connected=app->initialized=false;

	if(app->initialized && nErrorCode!=0)
		MessageBox(NULL,"Connection to WinLIRC server lost.","WinLIRC",MB_OK);
}
