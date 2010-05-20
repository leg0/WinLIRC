//=============================================================================
//  Copyright (C) 2004, Home Electronics ( http://www.home-electro.com )
//  support@home-electro.com
//
// This software is a part of Tira API. Permission is granted to use and modify
// as long as this notice is preserved, and the software remains a part of Tira API
// 
// No warranty express or implied.
//=============================================================================

#include <Windows.h>
#include "TiraDLL.h"

TiraDLL::TiraDLL() {

	dllHandle = NULL;
	loadDLL();
}

TiraDLL::~TiraDLL() {

	unloadDLL();
}

bool TiraDLL::loadDLL() {

	dllHandle = LoadLibraryA( ".\\plugins\\tira2.dll" );

	p_tira_init				= (t_tira_init)					GetProcAddress(dllHandle,"tira_init");
	p_tira_cleanup			= (t_tira_cleanup)				GetProcAddress(dllHandle,"tira_cleanup");
	p_tira_set_handler		= (t_tira_set_handler)			GetProcAddress(dllHandle,"tira_set_handler");;
	p_tira_start			= (t_tira_start)				GetProcAddress(dllHandle,"tira_start");;
	p_tira_stop				= (t_tira_stop)					GetProcAddress(dllHandle,"tira_stop");;
	p_tira_start_capture	= (t_tira_start_capture)		GetProcAddress(dllHandle,"tira_start_capture");;
	p_tira_cancel_capture	= (t_tira_cancel_capture)		GetProcAddress(dllHandle,"tira_cancel_capture");;
	p_tira_get_captured_data= (t_tira_get_captured_data)	GetProcAddress(dllHandle,"tira_get_captured_data");;
	p_tira_transmit			= (t_tira_transmit)				GetProcAddress(dllHandle,"tira_transmit");;
	p_tira_delete			= (t_tira_delete)				GetProcAddress(dllHandle,"tira_delete");;
	p_tira_set_dword		= (t_tira_set_dword)			GetProcAddress(dllHandle,"tira_set_dword");;
	p_tira_get_version		= (t_tira_get_version)			GetProcAddress(dllHandle,"tira_get_version");;
	p_tira_access_feature	= (t_tira_access_feature)		GetProcAddress(dllHandle,"tira_access_feature");;

	if(!dllHandle) return false;

	return true;
}

void TiraDLL::unloadDLL() {

	FreeLibrary(dllHandle);

	dllHandle = NULL;

	t_tira_init               p_tira_init				= NULL;
	t_tira_cleanup            p_tira_cleanup			= NULL;
	t_tira_set_handler        p_tira_set_handler		= NULL;
	t_tira_start              p_tira_start				= NULL;
	t_tira_stop               p_tira_stop				= NULL;
	t_tira_start_capture      p_tira_start_capture		= NULL;
	t_tira_cancel_capture     p_tira_cancel_capture		= NULL;
	t_tira_get_captured_data  p_tira_get_captured_data	= NULL;
	t_tira_transmit           p_tira_transmit			= NULL;
	t_tira_delete             p_tira_delete				= NULL;
	t_tira_set_dword          p_tira_set_dword			= NULL;
	t_tira_get_version        p_tira_get_version		= NULL;
	t_tira_access_feature     p_tira_access_feature		= NULL;
}

int TiraDLL::tira_init() {

	if(p_tira_init) return p_tira_init();

	return TIRA_FALSE;
}

int TiraDLL::tira_cleanup() {

	if(p_tira_cleanup) return p_tira_cleanup();

	return TIRA_FALSE;
}

int TiraDLL::tira_set_handler(tira_six_byte_cb cb) {

	if(p_tira_set_handler) return p_tira_set_handler(cb);

	return TIRA_FALSE;
}

int TiraDLL::tira_start(int PortID) {

	if(p_tira_start) return p_tira_start(PortID);

	return TIRA_FALSE;
}

int TiraDLL::tira_stop() {

	if(p_tira_stop) return p_tira_stop();
	
	return TIRA_FALSE;
}

int TiraDLL::tira_start_capture() {

	if(p_tira_start_capture) return p_tira_start_capture();

	return TIRA_FALSE;
}

int TiraDLL::tira_cancel_capture() {

	if(p_tira_cancel_capture) return p_tira_cancel_capture();

	return TIRA_FALSE;
}

int TiraDLL::tira_get_captured_data(const unsigned char **data, int *size) {

	if(p_tira_get_captured_data) return p_tira_get_captured_data(data,size);

	return TIRA_FALSE;
}

int TiraDLL::tira_transmit(int repeat, int frequency, const unsigned char *data, const int dataSize) {

	if(p_tira_transmit) return p_tira_transmit(repeat,frequency,data,dataSize);

	return TIRA_FALSE;
}

int TiraDLL::tira_delete(const unsigned char *ptr) {

	if(p_tira_delete) return p_tira_delete(ptr);

	return TIRA_FALSE;
}

int TiraDLL::tira_set_dword(unsigned int addr, unsigned int val) {

	if(p_tira_set_dword) return p_tira_set_dword(addr,val);

	return TIRA_FALSE;
}

const char* TiraDLL::tira_get_version(int component) {

	if(p_tira_get_version) return p_tira_get_version(component);

	return NULL;
}

int TiraDLL::tira_access_feature(unsigned int FeatureID, bool Write, unsigned int *Value, unsigned int Mask) {

	if(p_tira_access_feature) return p_tira_access_feature(FeatureID,Write,Value,Mask);

	return TIRA_FALSE;
}


