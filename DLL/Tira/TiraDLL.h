#ifndef TIRADLL_H
#define TIRADLL_H

#define TIRA_TRUE				0	//tira uses 0 as everything went okay
#define TIRA_FALSE				1	//tira false 
#define TIRA_NOT_IMPLEMENTED	2	//function not implimented

typedef int (__stdcall * tira_six_byte_cb)(const char * eventstring);
typedef int (__stdcall * t_tira_init)();
typedef int (__stdcall * t_tira_cleanup)();
typedef int (__stdcall * t_tira_set_handler)(tira_six_byte_cb cb);
typedef int (__stdcall * t_tira_start)(int PortID);
typedef int (__stdcall * t_tira_stop)();
typedef int (__stdcall * t_tira_start_capture)();
typedef int (__stdcall * t_tira_cancel_capture)();
typedef int (__stdcall * t_tira_get_captured_data)(const unsigned char** data, int* size );
typedef int (__stdcall * t_tira_transmit)(int repeat, int frequency, const unsigned char* data, const int dataSize );
typedef int (__stdcall * t_tira_delete)(const unsigned char* ptr);
typedef int (__stdcall * t_tira_set_dword)(unsigned int addr, unsigned int val);
typedef const char* (__stdcall * t_tira_get_version)(int component);
typedef int (__stdcall * t_tira_access_feature)(unsigned int FeatureID,bool Write,unsigned int* Value,unsigned int Mask);

class TiraDLL {

public:
	TiraDLL();
   ~TiraDLL();

	int tira_init				();
	int tira_cleanup			();
	int tira_set_handler		(tira_six_byte_cb cb);
	int tira_start				(int PortID);
	int tira_stop				();
	int tira_start_capture		();
	int tira_cancel_capture		();
	int tira_get_captured_data	(const unsigned char** data, int* size );
	int tira_transmit			(int repeat, int frequency, const unsigned char* data, const int dataSize );
	int tira_delete				(const unsigned char* ptr);
	int tira_set_dword			(unsigned int addr, unsigned int val);
	const char* tira_get_version(int component);
	int tira_access_feature		(unsigned int FeatureID,bool Write,unsigned int* Value,unsigned int Mask);

private:

	bool loadDLL();
	void unloadDLL();

	HMODULE						dllHandle;

	t_tira_init					p_tira_init;
	t_tira_cleanup				p_tira_cleanup;
	t_tira_set_handler			p_tira_set_handler;
	t_tira_start				p_tira_start;
	t_tira_stop					p_tira_stop;
	t_tira_start_capture		p_tira_start_capture;
	t_tira_cancel_capture		p_tira_cancel_capture;
	t_tira_get_captured_data	p_tira_get_captured_data;
	t_tira_transmit				p_tira_transmit;
	t_tira_delete				p_tira_delete;
	t_tira_set_dword			p_tira_set_dword;
	t_tira_get_version			p_tira_get_version;
	t_tira_access_feature		p_tira_access_feature;
};

#endif