#include "BeholdRC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



typedef ULONG (__cdecl GETCARDCOUNT) (void);              // Return the total number of Beholder WDM capture devices
typedef LPSTR (__cdecl GETCARDNAME) (IN ULONG ulCardIdx); // Return the device name by index ulCardIdx
typedef BOOL  (__cdecl OPENCARD) (IN ULONG ulCardIdx);    // Initialize device by index ulCardIdx
typedef ULONG (__cdecl GETREMOTECODE) (void);             // Return the key pressed on remote contril unit
typedef ULONG (__cdecl GETREMOTECODEEX) (void);           // Return the full 32-bit code of key pressed on remote contril unit
typedef BOOL  (__cdecl UNINIT) (void);                    // Free all resources



class BeholdRC
{
public:
  BeholdRC();
  ~BeholdRC();

public:
//private:
  GETREMOTECODEEX*  pGetRemoteCodeEx;
  GETREMOTECODE*    pGetRemoteCode;
  GETCARDCOUNT*     pGetCardCount;
  GETCARDNAME*      pGetCardName;
  OPENCARD*         pOpenCard;

  UNINIT*  pUnInit;
  HMODULE  hLib;
  bool  init;

} rc;



BeholdRC::BeholdRC() :
  pGetRemoteCodeEx(),
  pGetRemoteCode(),
  pGetCardCount(),
  pGetCardName(),
  pOpenCard(),
  pUnInit(),
  hLib(),
  init()
{
  init = false;

  hLib = LoadLibrary( _T( "BeholdRC.dll" ) );
  if ( hLib ) {
    pGetCardCount    = (GETCARDCOUNT*)    GetProcAddress( hLib, "GetCardCount" );
    pGetCardName     = (GETCARDNAME*)     GetProcAddress( hLib, "GetCardName" );
    pOpenCard        = (OPENCARD*)        GetProcAddress( hLib, "OpenCard" );
    pGetRemoteCode   = (GETREMOTECODE*)   GetProcAddress( hLib, "GetRemoteCode" );
    pGetRemoteCodeEx = (GETREMOTECODEEX*) GetProcAddress( hLib, "GetRemoteCodeEx" );
    pUnInit          = (UNINIT*)          GetProcAddress( hLib, "UnInit" );

    // GetRemoteCodeEx may be absent in library. Don't check for it.
    if( !pGetCardCount || !pGetCardName || !pOpenCard || !pGetRemoteCode || !pUnInit ) {
      FreeLibrary( hLib );
      hLib = nullptr;
    }
  }
}



BeholdRC::~BeholdRC()
{
  if( hLib ) {
    (*pUnInit)();
    FreeLibrary (hLib);
  }
}



int BTV_GetIStatus( void )
{
  if( rc.hLib == nullptr )
     return 0;	// Library not found.
  if( !rc.init )
     return 1;	// WDM device not selected.

  return 2;	// OK.
}



BOOL BTV_SelectCard( int idx )
{
  int  i, cnt;

  if( idx>=0 && rc.hLib ) {
     cnt = (*rc.pGetCardCount)();
     if( cnt>idx ) {
        for( i=0; i<cnt; ++i ) {
           if( (*rc.pOpenCard)(i) ) {
              if( idx==0 ) {
                 rc.init = true;
                 return TRUE;
              }
              --idx;
           }
        }
     }
  }
  rc.init = false;
  return FALSE;
}



int BTV_GetRCCode( void )
{
  int  code;
  if( rc.hLib && rc.init ) {
    code = (*rc.pGetRemoteCode)();
    if( code<255 )
       return code;
  }
  return -1;
}



ULONG BTV_GetRCCodeEx( void )
{
  ULONG  code;
  if( rc.hLib && rc.init ) {
    if( rc.pGetRemoteCodeEx )
       return (*rc.pGetRemoteCodeEx)();
    code = (*rc.pGetRemoteCode)();	// The BeholdTV software has old BeholdRC.dll. Emulate extended function
    if( code<255 )
       return 0x866B0000 | (code<<8) | (~code&0xFF);	// Combine full NEC-standard code
  }
  return 0;
}
