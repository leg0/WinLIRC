//
#ifndef TTUSBIRAPIEXP_INCD
#define TTUSBIRAPIEXP_INCD

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifdef TTUSBIR_EXPORTS
#ifdef __cplusplus 
#define TTUSBIR_API extern "C" __declspec(dllexport)
#else 
#define TTUSBIR_API __declspec(dllexport)
#endif 
#else
#ifdef __cplusplus 
#define TTUSBIR_API extern "C" __declspec(dllimport)
#else 
#define TTUSBIR_API __declspec(dllimport)
#endif
#endif

// You must define TTUSBIR_STATIC_LIBRARY in your project if you use
// the static (not DLL) version of the library!
#ifdef TTUSBIR_STATIC_LIBRARY
#undef TTUSBIR_API
#ifdef __cplusplus 
#define TTUSBIR_API extern "C"
#else 
#define TTUSBIR_API
#endif
#endif


//--- High Level Infrared Receiver Interface --------------------------------

	typedef enum _USBIR_MODES
	{
		USBIR_MODE_NONE,
		USBIR_MODE_RAW,
		USBIR_MODE_DIV,
		USBIR_MODE_ALL_CODES, // special mode for decting the type of a remote control
		USBIR_MODE_RC5,
		USBIR_MODE_NOKIA,
		USBIR_MODE_NEC,
		USBIR_MODE_RCMM8, //USBIR_MODE_RCMM,
		USBIR_MODE_RC6, // RC6 Mode 6A
		USBIR_MODE_RECS80,
		USBIR_MODE_DENON, // = Sharp
		USBIR_MODE_MOTOROLA,
		USBIR_MODE_SONYSIRC
	} USBIR_MODES;

	#define USBIR_MODE_MAX USBIR_MODE_SONYSIRC

	// Buf is a BYTE-Buffer containing bit samples in case of USBIR_MODE_RAW mode
	// Buf is a DWORD-Buffer containing IRCodes in case of all other modes, exept USBIR_MODE_ALL_CODES
	// Buf is a __int64-Buffer containing pairs of 32Bit IRCode and 32Bit IRMode in case of USBIR_MODE_ALL_CODES
	typedef void (*PIRCBFCN) (PVOID Context, PVOID Buf, ULONG len, // buffer length in bytes
								USBIR_MODES IRMode, HANDLE hOpen, BYTE DevIdx);

	TTUSBIR_API BYTE	irGetNumDevices (void);

	TTUSBIR_API HANDLE	irOpen	(int DevIdx, USBIR_MODES IRMode, 
									PIRCBFCN CallbackFcn = NULL, PVOID Context = NULL);

	TTUSBIR_API void	irClose	(HANDLE hOpen);

	TTUSBIR_API int		irRead	(HANDLE hOpen, PVOID Buf, ULONG len,
									DWORD msTimeout = INFINITE); // INFINITE == Blocking-Mode

	TTUSBIR_API int		irGetDevIdx (HANDLE hOpen);


//--- Low Level Infrared Receiver Interface ---------------------------------

	TTUSBIR_API int		irUpdateSTC			(HANDLE hOpen, BYTE* Image, WORD ImageLen); // ((ret==0)||(ret==9))?success:failure

	TTUSBIR_API BOOL	irSetBusyLEDFreq	(HANDLE hOpen, BYTE freq);
	TTUSBIR_API BOOL	irSetPowerLED		(HANDLE hOpen, BOOL ONoff);

	TTUSBIR_API BOOL	irGetDriverVersion	(HANDLE hOpen, DWORD& dwDrvVer);
	TTUSBIR_API BOOL	irGetSTCVersion		(HANDLE hOpen, char** strVersion);
	
	TTUSBIR_API BOOL	irGetDriverStatus	(HANDLE hOpen, DWORD* pdwArr, BYTE NrOfDWs); // expects 20 DWORD Buffer
	TTUSBIR_API BOOL	irGetSTCStatus		(HANDLE hOpen, BYTE* pBuf, int NrOfBytes); // expects 28 BYTES Buffer
	
	// Warning: Usage of the two functions below without detailed knowledge can destroy the device
	TTUSBIR_API BOOL	irSTCSetGPIO		(HANDLE hOpen, BYTE Addr, BYTE Data); // Addr: 1 for whole Port1; 10 to 17 for Bit 0 to 7 of Port1
	TTUSBIR_API BOOL	irSTCGetGPIO		(HANDLE hOpen, BYTE Addr, BYTE& Data); // Addr: 1 for whole Port1; 10 to 17 for Bit 0 to 7 of Port1
	
	// Warning: Usage of the four functions below without detailed knowledge can destroy the device
    TTUSBIR_API BOOL	irI2cSetBitRate		(HANDLE hOpen, WORD nBitRate); // I2C Bit-Rate in kHz (100 or 400)
    TTUSBIR_API BOOL	irI2cRead			(HANDLE hOpen, BYTE slave, BYTE* seq, BYTE len);  // read  some bytes from I2C
    TTUSBIR_API BOOL	irI2cWrite			(HANDLE hOpen, BYTE slave, BYTE* seq, BYTE len);  // write some bytes to I2C
    TTUSBIR_API BOOL	irI2cCombined		(HANDLE hOpen, BYTE slave, BYTE* seqWr, BYTE lenWr, BYTE* seqRd, BYTE lenRd); // read after write


//--- Useful helper functions -----------------------------------------------

	// Uniting function for all IR-Modes (DIV,RC5,NOKIA,NEC,RCMM,RC6,RECS80,DENON,MOTOROLA,SONYSIRC)
	// converts an IRMode dependent IRCode into a unique code
	TTUSBIR_API DWORD ir_GetUniqueCode(DWORD IRCode, USBIR_MODES IRMode);

	//returns a description of the given IR mode
	const char* ir_GetIRModeDescription(USBIR_MODES IRMode);

	// Functions for RCMM Mode
	// (0) RCMM-BASIC-Modus:    2 Bit Modus, 2 Bit Address, 8 Bit Data
	// (1) RCMM-EXTENDED-Modus: 6 Bit Modus, 20 Bit Data incl. Address
	// (2) RCMM-OEM-Modus:      6 Bit Modus, 6 Bit CostumerID, 12 Bit Data incl. Address
	// (3) RCMM-OEM_LONG-Modus: 6 Bit Modus, 6 Bit CostumerID=010101, 20 Bit Data incl. Address
	TTUSBIR_API BYTE  ir_Get_RCMM_Mode(DWORD IRCode);
	TTUSBIR_API DWORD ir_Get_RCMM_Data(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_RCMM_Address(DWORD IRCode); // Mode 0 only
	TTUSBIR_API BYTE  ir_Get_RCMM_CostumerID(DWORD IRCode); // Mode 2, 3 only
	
	// Functions for NEC Mode
	TTUSBIR_API BYTE  ir_Get_NEC_Command(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_NEC_Address(DWORD IRCode);

	// Functions for Nokia Mode
	TTUSBIR_API BYTE  ir_Get_NOKIA_Command(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_NOKIA_Address(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_NOKIA_SubCode(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_NOKIA_LowBattery(DWORD IRCode);
	
	// Functions for RC-5 Mode
	TTUSBIR_API BYTE  ir_Get_RC5_Command(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_RC5_ExtCommand(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_RC5_Address(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_RC5_ToggleBit(DWORD IRCode);

	// Functions for Sharp Mode	
	TTUSBIR_API BYTE  ir_Get_Sharp_Address(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_Sharp_KeyCode(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_Sharp_Expansion(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_Sharp_Check(DWORD IRCode);

	// Functions for Denon Mode	
	TTUSBIR_API BYTE  ir_Get_Denon_Address(DWORD IRCode);
	TTUSBIR_API WORD  ir_Get_Denon_KeyCode(DWORD IRCode);
	
	// Functions for RECS80 Mode	
	TTUSBIR_API BYTE  ir_Get_RECS80_Address(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_RECS80_KeyCode(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_RECS80_ToggleBit(DWORD IRCode);

	// Functions for SonySIRC 12, 15 and 20 Bit Mode (Mode 0, 1 und 2)
	TTUSBIR_API BYTE  ir_Get_SIRC_Mode(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_SIRC_Address(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_SIRC_KeyCode(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_SIRC_Extension(DWORD IRCode);

	// Functions for RC-6 Mode 6A
	TTUSBIR_API WORD  ir_Get_RC6_Information(DWORD IRCode);
	TTUSBIR_API WORD  ir_Get_RC6_HeaderData(DWORD IRCode);

	// Functions for Motorola Mode
	TTUSBIR_API BYTE  ir_Get_Motorola_Data(DWORD IRCode);
	
	// Functions for All Codes Mode
	TTUSBIR_API DWORD		ir_Get_AllCode_IRCode(__int64 IRCodeIRMode);
	TTUSBIR_API USBIR_MODES	ir_Get_AllCode_IRMode(__int64 IRCodeIRMode);

	// Functions for Div-Mode
	TTUSBIR_API DWORD ir_Get_DIV_TimeStamp(DWORD IRCode);
	TTUSBIR_API BYTE  ir_Get_DIV_BitValue(DWORD IRCode);

//--- Useful helper macros --------------------------------------------------

	#define SWAP3(a)	( ((a>>2)&0x01) | ((a)&0x02)	| ((a<<2)&0x04) )
	#define SWAP4(a)	( ((a>>3)&0x01) | ((a>>1)&0x02) | ((a<<1)&0x04) | ((a<<3)&0x08) )
	#define SWAP5(a)	( ((a>>4)&0x01) | ((a>>2)&0x02) | ((a)&0x04)	| ((a<<2)&0x08) | ((a<<4)&0x10) )
	#define SWAP7(a)	( ((a>>6)&0x01) | ((a>>4)&0x02) | ((a>>2)&0x04) | ((a)&0x08)	| ((a<<2)&0x10) | ((a<<4)&0x20) | ((a<<6)&0x40) )
	#define SWAP8(a)	( ((a>>7)&0x01) | ((a>>5)&0x02) | ((a>>3)&0x04) | ((a>>1)&0x08) | ((a<<1)&0x10) | ((a<<3)&0x20) | ((a<<5)&0x40) | ((a<<7)&0x80) )
	#define SWAP10(a)	( ((a>>9)&0x01) | ((a>>7)&0x02) | ((a>>5)&0x04) | ((a>>3)&0x08) | ((a>>1)&0x10)	| ((a<<1)&0x20) | ((a<<3)&0x40) | ((a<<5)&0x80) | ((a<<7)&0x100) | ((a<<9)&0x200) )

	// Macros for RCMM Mode
	// (0) RCMM-BASIC-Modus:    2 Bit Modus, 2 Bit Address, 8 Bit Data
	// (1) RCMM-EXTENDED-Modus: 6 Bit Modus, 20 Bit Address+Data
	// (2) RCMM-OEM-Modus:      6 Bit Modus, 6 Bit CostumerID, 12 Bit Address+Data
	// (3) RCMM-OEM_LONG-Modus: 6 Bit Modus, 6 Bit CostumerID=010101, 20 Bit Address+Data
	#define RCMM_MODE(a)		(((a>=0x0D500000) ? 3 : (a>=0x00100000) ? 1 : (a>=0x000C0000) ? 2 : 0))
	#define RCMM_DATA_BASIC(a)	(a & 0x000000ff)
	#define RCMM_DATA_EXT(a)	(a & 0x000fffff)
	#define RCMM_DATA_OEM(a)	(a & 0x00000fff)
	#define RCMM_DATA_OEM_L(a)	(a & 0x000fffff)
	#define RCMM_ADDR_BASIC(a)	((a >> 8) & 0x00000003)
	#define RCMM_ID_OEM(a)		((a >> 12) & 0x0000003f)
	#define RCMM_ID_OEM_L(a)	((a >> 20) & 0x0000003f)

	// Macros for NEC Mode
	#define NEC_COMMAND(a)		(SWAP8((a & 0x000000ff)))
	#define NEC_ADDRESS(a)		(SWAP8((a & 0x0000ff00) >> 8))

	// Macros for Nokia Mode
	#define NOKIA_LOWBATTERY(a)	((a & 0x00020000) >> 17)
	#define NOKIA_COMMAND(a)	(SWAP8((a & 0x0000ff00) >> 8))
	#define NOKIA_ADDRESS(a)	(SWAP4((a & 0x000000f0) >> 4))
	#define NOKIA_SUBCODE(a)	(SWAP4((a & 0x0000000f)))

	// Macros for RC-5 Mode
	#define RC5_COMMAND(a)		(a & 0x0000003f)
	#define RC5_EXTCOMMAND(a)	((a & 0x0000003f) | (((a >> 6) & 0x00000040) ^ 0x00000040))
	#define RC5_ADDRESS(a)		((a >> 6) & 0x0000001f)
	#define RC5_TOGGLEBIT(a)	((a >> 11) & 0x00000001)

	// Macros for Sharp Mode	
	#define SHARP_DEVADDR(a)	(SWAP5((a & 0x00007c00) >> 10))
	#define SHARP_KEYCODE(a)	(SWAP8((a & 0x000003fc) >> 2))
	#define SHARP_EXPANSION(a)	((a & 0x00000002) >> 1)
	#define SHARP_CHECK(a)		(a & 0x00000001)
	
	// Macros for Denon Mode	
	#define DENON_DEVADDR(a)	(SWAP5((a & 0x00007c00) >> 10))
	#define DENON_KEYCODE(a)	(SWAP10((a & 0x000003ff)))

	// Macros for RECS80 Mode	
	#define RECS80_DEVADDR(a)	(SWAP3((a & 0x00000380) >> 7))
	#define RECS80_KEYCODE(a)	((a & 0x0000007e) >> 1)
	#define RECS80_TOGGLEBIT(a)	((a & 0x00000400) >> 10)

	// Macros for SonySIRC 12, 15 and 20 Bit Mode (Mode 0, 1 und 2)
	#define SONYSIRC_MODE(a)		(((a >> 20) ? 2 : (a >> 15) ? 1 : 0))
	#define SONYSIRC12_KEYCODE(a)	(SWAP7((a & 0x00000fe0) >> 5))
	#define SONYSIRC12_DEVADDR(a)	(SWAP5((a & 0x0000001f)))
	#define SONYSIRC15_KEYCODE(a)	(SWAP7((a & 0x00007f00) >> 8))
	#define SONYSIRC15_DEVADDR(a)	(SWAP5((a & 0x000000f8) >> 3))
	#define SONYSIRC15_EXTENSN(a)	(SWAP3((a & 0x00000003)))
	#define SONYSIRC20_KEYCODE(a)	(SWAP7((a & 0x0001fc00) >> 10))
	#define SONYSIRC20_DEVADDR(a)	(SWAP5((a & 0x000003e0) >> 5))
	#define SONYSIRC20_EXTENSN(a)	(SWAP5((a & 0x0000001f)))

	// Macros for RC-6 Mode 6A
	#define RC6_INFORMATION(a)	(a & 0x0000ffff)
	#define RC6_HEADERDATA(a)	((a >> 16) & 0x0000ffff)

	// Macros for Motorola Mode
	#define MOTOROLA_DATA(a)	(a & 0x000001ff)

	// Macros for All Codes Mode
	#define ALL_CODES_IRCODE(a)	(DWORD(a >> 32))
	#define ALL_CODES_IRMODE(a)	(USBIR_MODES(a & 0xffffffff))

	// Macros for Div-Mode
	#define DIV_TIMESTAMP(a)	(a & 0x00ffffff)
	#define DIV_BITVALUE(a)		((a >> 24) & 0x00000001)


#endif // TTUSBIRAPIEXP_INCD
