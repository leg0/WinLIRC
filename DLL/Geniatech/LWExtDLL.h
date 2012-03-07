#ifndef _LWEXT_H_
#define _LWEXT_H_

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LWEXTDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LWEXTDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#define DLL_ERROR	0xff
#define DLL_OK		0x00

//New Func without lwext.cfg.
int  LWEXT_OpenEx(char *chID);
int  LWEXT_Open(ULONG ulID);

int  LWEXT_Close(void);

int  LWEXT_GetIr(ULONG *pulCommand, ULONG *pulAddress);

int  MYHDEXT_ReadI2C(UCHAR ucChip, ULONG ulSubAddr, UCHAR ucSubSize, ULONG ulCount, UCHAR *ucData);
int  MYHDEXT_WriteI2C(UCHAR ucChip, ULONG ulSubAddr, UCHAR ucSubSize, ULONG ulCount, UCHAR *ucData);
int  MYHDEXT_BurnSecurity(UCHAR ucID);

int  MYHDEXT_GetFriendlyName(PBYTE pBuf, ULONG ulSize);

//For mac address.
int  LWEXT_SetMacEeAddr(BYTE ucChip, ULONG ulAddr);
int  LWEXT_GetMacAddr(PBYTE pBuf, ULONG ulSize);
int  LWEXT_SetMacAddr(PBYTE pBuf, ULONG ulSize);

#endif
