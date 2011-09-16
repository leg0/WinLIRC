/*

	Copyrights (c) TeVii Technology.
	Unauthorized usage is prohibited.

*/

#pragma once

#ifdef TEVII_EXPORTS
#define TEVII_API extern "C" __declspec(dllexport)
#else
#define TEVII_API extern "C" __declspec(dllimport)
#endif


enum TFEC
{
	TFEC_AUTO,
	TFEC_1_2,
	TFEC_1_3,
	TFEC_1_4,
	TFEC_2_3,
	TFEC_2_5,
	TFEC_3_4,
	TFEC_3_5,
	TFEC_4_5,
	TFEC_5_6,
	TFEC_5_11,
	TFEC_6_7,
	TFEC_7_8,
	TFEC_8_9,
	TFEC_9_10
};

enum TMOD
{
	TMOD_AUTO,
	TMOD_QPSK,
	TMOD_BPSK,
	TMOD_16QAM,
	TMOD_32QAM,
	TMOD_64QAM,
	TMOD_128QAM,
	TMOD_256QAM,
	TMOD_8VSB,
	TMOD_DVBS2_QPSK,
	TMOD_DVBS2_8PSK,
	TMOD_DVBS2_16PSK,
	TMOD_DVBS2_32PSK,
	TMOD_TURBO_QPSK,
	TMOD_TURBO_8PSK,
	TMOD_TURBO_16PSK
};

enum TPolarization
{
	TPol_None,
	TPol_Vertical,
	TPol_Horizontal
};

// Capture function supplied by application
typedef void (__stdcall *lpCapFunc)(void* lParam, BYTE* Buffer, int Length);

//application supplied function which will receive keys from remote control
typedef void (__stdcall *lpRCFunc)(void* lParam, DWORD key);

//////////////////////////////////////////////////////////////////////////
// Information functions
// these functions don't require opening of device
//

// get version of SDK API
// This is optional.
// Can be used to check if API is not lower than originally used.
TEVII_API int GetAPIVersion();

//Enumerate drvices in system.
//This function should be called before any other.
//Only first call will really enumerate. All subsequent 
//calls will just return number of previously found devices.
//	result: number of found devices
TEVII_API int FindDevices();

//Get name of device
//	params:
//		idx - device index (0 <= idx < FindDevices())
//	result: string with device name or NULL (on failure). Do not modify or free memory used by this string!
TEVII_API const char* GetDeviceName(int idx);

//Get device path
//	params:
//		idx - device index (0 <= idx < FindDevices())
//	result: string with device path or NULL (on failure). Do not modify or free memory used by this string!
TEVII_API const char* GetDevicePath(int idx);


//////////////////////////////////////////////////////////////////////////
// Following functions work only after call OpenDevice()
//

//Open device. Application may open several devices simultaneously. 
//They will be distinguished by idx parameter.
//	params:
//		idx - device index (0 <= idx < FindDevices())
//		func - capture function which will receive stream.
//		lParam - application defined parameter which will be passed to capture function
//	result: non-zero on success
TEVII_API BOOL  OpenDevice(int idx, lpCapFunc func, void* lParam);

//Close Device
//	params:
//		idx - device index of previously opened device (0 <= idx < FindDevices())
//	result: non-zero on success
TEVII_API BOOL  CloseDevice(int idx);

//Tune to transponder
//	params:
//		idx - device index of previously opened device (0 <= idx < FindDevices())
//		Freq - frequency in kHz, for example: 12450000 (12.45 GHz)
//		SymbRate - Symbol rate in sps, for example: 27500000 (27500 Ksps)
//		Pol - polarization, see TPolarization above
//		F22KHz - 22KHz tone on/off
//		MOD - modulation, see TMOD above. Note: it's better to avoid use AUTO for DVBS2 signal, otherwise locking time will be long
//		FEC - see TFEC above. Note: it's better to avoid use AUTO for DVBS2 signal, otherwise locking time will be long
//	result: non-zero if signal is locked
TEVII_API BOOL  TuneTransponder(int idx, DWORD Freq, DWORD SymbRate, DWORD LOF, TPolarization Pol, BOOL  F22KHz, TMOD MOD, TFEC FEC);

//Get signal status
//	params:
//		idx - device index of previously opened device (0 <= idx < FindDevices())
//		isLocked - non-zero if signal is present
//		Strength - 0..100 signal strength
//		Quality - 0..100 signal quality
//	result: non-zero on success
TEVII_API BOOL  GetSignalStatus(int idx, BOOL* IsLocked, DWORD* Strength, DWORD* Quality);

//Send DiSEqC message
//	params:
//		idx - device index of previously opened device (0 <= idx < FindDevices())
//		Data,Len - buffer with DiSEqC command
//		Repeats - repeat DiSEqC message n times. 0 means send one time
//		Flg - non-zero means replace first byte (0xE0) of DiSEqC message with 0xE1 on second and following repeats.
//	result: non-zero on success
TEVII_API BOOL SendDiSEqC(int idx, BYTE* Data, DWORD Len, DWORD Repeats, BOOL Flg);

//Send DiSEqC message
//	params:
//		idx - device index of previously opened device (0 <= idx < FindDevices())
//		lpCallback - application defined procedure to receive keys. NULL to disable RC.
//		lParam - application defined parameter which will be passed to callback function
//	result: non-zero on success
TEVII_API BOOL SetRemoteControl(int idx, lpRCFunc lpCallback, void* lParam);
