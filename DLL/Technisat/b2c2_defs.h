/*-------------------------------------------------------------------------------------

	Filename:	b2c2_defs.h

	Description:	

	History:		

		12/29/03	Add enumeration for Bandwidth required for DVB-T tuning.
		06/09/04	Added support for full transport stream capture. [JJ]
		09/21/05	Added DiSEqC 1.2 support [ARS]
		11/18/05	Add device list support. [ARS]
		12/20/05	Increase maximum number of supported devices to 16. [ARS]
		12/15/06	Added support of IR input. [ARS]

	Copyright (c) 1998-2004, B2C2, Inc.

	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF B2C2, INC. The copyright notice 
	above does not evidence any actual or intended publication of such source code.

	This file is released pursuant to and subject to the restrictions of the 
	non-disclosure agreement and license contract entered into by the parties.

---------------------------------------------------------------------------------------*/

#ifndef _B2C2_DEFS_H_
#define _B2C2_DEFS_H_

/* 
 *	FEC enumeration used by SetFEC()/GetFEC()
 */
typedef enum eFECTAG
{
	FEC_1_2 = 1,
	FEC_2_3,
	FEC_3_4,
	FEC_5_6,
	FEC_7_8,
	FEC_AUTO,
	FEC_COUNT = 6,
} eFEC;

/* 
 *	Guard interval enumeration used by SetGuardInterval()/GetGuardInterval()
 */
typedef enum eGuardIntervalTAG
{
	GUARD_INTERVAL_1_32 = 0,
	GUARD_INTERVAL_1_16,
	GUARD_INTERVAL_1_8,
	GUARD_INTERVAL_1_4,
	GUARD_INTERVAL_AUTO,
	GUARD_INTERVAL_COUNT,
} eGuardInterval;

/* 
 *	Bandwidth enumeration used by SetBandwidth()/GetBandwidth()
 */
typedef enum eBandWidthTAG
{
	BANDWIDTH_6_MHZ = 6,
	BANDWIDTH_7_MHZ = 7,
	BANDWIDTH_8_MHZ = 8,
	BANDWIDTH_COUNT = 3,
	BANDWIDTH_UNKNOWN = -1
} eBandwidth;

/* 
 *	Polarity enumeration used by SetPolarity()/GetPolarity ()
 */
typedef enum ePolarityTAG
{
	POLARITY_HORIZONTAL = 0,
	POLARITY_VERTICAL,
	POLARITY_COUNT,
 	//if no LNB power is needed
	POLARITY_LNB_NO_POWER = 10,
} ePolarity;

/* 
 *	LNB enumeration used by SetLnbKHz()/GetLnbKHz()
 */
typedef enum eLNBSelectionTAG
{
	LNB_SELECTION_0 = 0,
	LNB_SELECTION_22,
	LNB_SELECTION_33,
	LNB_SELECTION_44,
	LNB_SELECTION_COUNT,
} eLNBSelection;

/* 
 *	Diseqc enumeration used by SetDiseqc()/GetDiseqc()
 */
typedef enum eDiseqcTAG
{
	DISEQC_NONE = 0,
	DISEQC_SIMPLE_A,
	DISEQC_SIMPLE_B,
	DISEQC_LEVEL_1_A_A,
	DISEQC_LEVEL_1_B_A,
	DISEQC_LEVEL_1_A_B,
	DISEQC_LEVEL_1_B_B,
	DISEQC_COUNT
} eDiseqc;

/* 
 *	Modulation enumeration used by SetModulation()/GetModulation()
 */
typedef enum eModulationTAG
{
	_QPSK = 0,
	_8PSK,
	QAM_4,
	QAM_16,
	QAM_32,
	QAM_64,
	QAM_128,
	QAM_256,
	QAM_64_ANNEX_B,
	QAM_256_ANNEX_B,
	VSB_8,
	VSB_16,
	MODE_UNKNOWN = -1
} eModulation;

/* 
 *	Tuner Modulation enumeration used in TunerCapabilities structure to 
 *  return the modulation used by the tuner.
 */
typedef enum tTunerModulationTAG
{
	TUNER_SATELLITE = 0,
	TUNER_CABLE  = 1,
	TUNER_TERRESTRIAL = 2,
	TUNER_ATSC = 3,
	TUNER_TERRESTRIAL_DVB = TUNER_TERRESTRIAL,
	TUNER_TERRESTRIAL_ATSC = TUNER_ATSC,
	TUNER_UNKNOWN = -1,
} tTunerModulation;


/*
 *	Structure completedy by GetTunerCapabilities() to return tuner capabilities
 */
typedef struct tTunerCapabilities
{
	tTunerModulation	eModulation;
	unsigned long		dwConstellationSupported;       // Show if SetModulation() is supported
	unsigned long		dwFECSupported;                 // Show if SetFec() is suppoted
	unsigned long		dwMinTransponderFreqInKHz;
	unsigned long		dwMaxTransponderFreqInKHz;
	unsigned long		dwMinTunerFreqInKHz;
	unsigned long		dwMaxTunerFreqInKHz;
	unsigned long		dwMinSymbolRateInBaud;
	unsigned long		dwMaxSymbolRateInBaud;
	unsigned long		bAutoSymbolRate;				// Obsolete		
	unsigned long		dwPerformanceMonitoring;        // See bitmask definitions below
	unsigned long		dwLockTimeInMilliSecond;		// lock time in millisecond
	unsigned long		dwKernelLockTimeInMilliSecond;	// lock time for kernel
	unsigned long		dwAcquisitionCapabilities;
} tTunerCapabilities, *pTunerCapabilities;

/*
 *	Bitmasks for comparison with dwPerformanceMonitoring member of tTunerCapabilities
 *	to determine which of these various options are supported by the current tuner.
 */
															// If set in dwPerformanceMonitoring, tuner supports:
#define BER_SUPPORTED						1L				// BER reporting via GetPreErrorCorrectionBER ()
#define BLOCK_COUNT_SUPPORTED				(1L << 1)		// Block count report via GetTotalBlocks ()
#define CORRECTED_BLOCK_COUNT_SUPPORTED		(1L << 2)		// Corrected block count via GetCorrectedBlocks 
#define UNCORRECTED_BLOCK_COUNT_SUPPORTED	(1L << 3)		// Uncorrected block count via GetUncorrectedBlocks 
#define	SNR_SUPPORTED						(1L << 4)		// SNR via GetSNR ()
#define SIGNAL_STRENGTH_SUPPORTED			(1L << 5)		// Signal strength via GetSignalStrength()
#define SIGNAL_QUALITY_SUPPORTED			(1L << 6)		// Signal quality via GetSignalQuality()

/*
 *               **** Acquisition Capabilities flags. ****
 *
 */
#define ACQUISITION_AUTO_SYMBOL_RATE			1L			// DVB-S Automatic Symbol Rate detection
#define ACQUISITION_AUTO_GUARD_INTERVAL			(1L << 1)	// DVB-T Automatic Guard Interval detection
#define ACQUISITION_AUTO_FREQ_OFFSET			(1L << 2)	// DVB-T Automatic Frequency offset handling
#define ACQUISITION_VHF_SUPPORT					(1L << 3)	// DVB-T VHF support
#define ACQUISITION_BANDWIDTH_6MHZ				(1L << 4)	// DVB-T Support for 6MHz Bandwidth
#define ACQUISITION_BANDWIDTH_7MHZ				(1L << 5)	// DVB-T Support for 7MHz Bandwidth
#define ACQUISITION_BANDWIDTH_8MHZ				(1L << 6)	// DVB-T Support for 8MHz Bandwidth
#define ACQUISITION_DISEQC_12					(1L << 7)	// DiSEqC 1.2 support
#define ACQUISITION_IR_INPUT_SUPPORT			(1L << 8)	// IR input support

/*
 *	Structure for the available device information list
 */

#define ETH_LENGTH_OF_ADDRESS		6

#define MAX_DEVICES_SUPPORTED		16

typedef enum tBusInterfaceTAG
{
	DEVICE_INTERFACE_PCI = 0,
	DEVICE_INTERFACE_USB_1_1 = 1
} tBusInterface;

typedef struct tagDEVICE_INFORMATION
{
	unsigned long    dwDeviceID;
	unsigned char    ucMACAddress[ETH_LENGTH_OF_ADDRESS];
	tTunerModulation eTunerModulation;
	tBusInterface    eBusInterface;
	unsigned short	 wInUse;
	unsigned long	 dwProductID;
	wchar_t			 wsProductName[31];
	wchar_t			 wsProductDescription[81];
	wchar_t			 wsProductRevision[21];
	wchar_t			 wsProductFrontEnd[61];
} tDEVICE_INFORMATION, *PDEVICE_INFORMATION;

/*
 *	Structure for Mac address list used by *UnicastMacAddress* functions
 */
#define B2C2_SDK_MAC_ADDR_SIZE			6
#define B2C2_SDK_MAC_ADDR_LIST_MAX		32

#define MAX_IP_STR_LEN					16

typedef struct tMacAddressList
{
	long lCount;				// Input : Number of MAC addresses at array
								// Output: Number of MAC addresses set
	unsigned char aabtMacAddr[B2C2_SDK_MAC_ADDR_LIST_MAX][B2C2_SDK_MAC_ADDR_SIZE];
} tMacAddressList, *ptMacAddressList;

/*
 *	Datagram Table ID enumeration used by SetTableId()/GetTableId()
 */
typedef enum eTableIdTag
{
	TABLE_ID_3E = 0x3E,		// DVB Standard
	TABLE_ID_3F = 0x3F,		// ATSC Standard
	TABLE_ID_AUTO = 0xFF,	// Automatic
} eTableId;


/*
 *	Definitions for scrambling control key functions
 */
#define B2C2_SDK_FIXED_KEY_SIZE			8

typedef enum eKeyTypeTAG
{
	// PID-TSC Keys					// up to 7 Keys possible; highest priority 
	B2C2_KEY_PID_TSC_01 = 1,		//  Key is used if the PID matches and the TSC bits are '01' (reserved). 
	B2C2_KEY_PID_TSC_10 = 2,		//  Key is used if the PID matches and the TSC bits are '10' (even). 
	B2C2_KEY_PID_TSC_11 = 3,		//  Key is used if the PID matches and the TSC bits are '11' (odd). 
	// PID-only Key					// 1 key only; if no Global Key is set 
	B2C2_KEY_PID_TSC_ANY,			//  Key is used if no B2C2_KEY_PID_TSC_XX key matches and the PID value matches. 
	// Global Key					// 1 key only; if no PID-only Key is set 
	B2C2_KEY_GLOBAL,				//  Key is used if no B2C2_KEY_PID_TSC_XX key matches. 
} eKeyType;

/*
 *  Error codes, returned by B2C2 SDK functions in addition to 
 *  COM error codes.
 */

#define B2C2_SDK_E_ALREADY_EXIST			0x10011000		// The PID to add by AddPIDsToPin or AddPIDs already exists.
//#define B2C2_SDK_E_PID_ERROR				0x90011001
#define B2C2_SDK_E_ALREADY_FULL				0x90011002		// Failed to add PID by AddPIDsToPin or AddPIDs because maximum number reached.

// B2C2MPEG2Adapter error codes

#define B2C2_SDK_E_CREATE_INTERFACE 		0x90020001		// Not all interfaces could be created correctly.  
#define B2C2_SDK_E_UNSUPPORTED_DEVICE		0x90020002		// (Linux) The given network device is no B2C2 Boradband device.

#define B2C2_SDK_E_NOT_INITIALIZED 			0x90020003		// Device has not been initialized before calling this functions.
															// Call Initialize () first.

#define B2C2_SDK_E_INVALID_PIN	 			0x90020004		// (Windows) The pin number given at the first argument is invalid { 0 ... 3 }.
#define B2C2_SDK_E_NO_TS_FILTER				0x90020005		// (Windows) No custom renderer filter created. Call CreateTsFilter () first.
#define B2C2_SDK_E_PIN_ALREADY_CONNECTED	0x90020007		// (Windows) The output pin is already connected to a renderer filter input pin.
#define B2C2_SDK_E_NO_INPUT_PIN				0x90020008		// (Windows) No input pin on the custom renderer filter found, check pin name if given.
#define B2C2_SDK_E_INVALID_TID	 			0x90020009		// Invalid Table ID value was used at SetTableId call

#define B2C2_SDK_E_SET_GLOBAL_FIXED_KEY		0x9002000A		// 
#define B2C2_SDK_E_SET_PID_FIXED_KEY		0x9002000B		// 
#define B2C2_SDK_E_GET_PID_FIXED_KEYS		0x9002000C		// 
#define B2C2_SDK_E_DELETE_PID_FIXED_KEY		0x9002000D		// 
#define B2C2_SDK_E_PURGE_FIXED_KEY			0x9002000E		// 

#define B2C2_SDK_E_DISEQC_IN_PROGRESS		0x9002000F		// 
#define B2C2_SDK_E_DISEQC_12_NOT_SUPPORTED	0x90020010		// 

#define B2C2_SDK_E_NO_DEVICE_AVAILABLE		0x90020011		// 

typedef enum
{
	PID_CAPTURE_ALL_INCLUDING_NULLS = 0x2000,
	PID_CAPTURE_ALL_EXCLUDING_NULLS = 0x2001,
} tSpecialPids;

#endif	// _B2C2_DEFS_H_
