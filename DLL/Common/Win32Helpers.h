#ifndef _WIN32_HELPERS_H_
#define _WIN32_HELPERS_H_

#ifndef SAFE_CLOSE_HANDLE
#define SAFE_CLOSE_HANDLE(a) if(a!=NULL) { CloseHandle(a); a = NULL; }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(a) if(a!=NULL) { a->Release(); a = NULL; }
#endif

#ifndef SAFE_CLOSE_SOCKET
#define SAFE_CLOSE_SOCKET(a) if(a!=INVALID_SOCKET) { closesocket(a); a = INVALID_SOCKET; }
#endif

void KillThread(HANDLE exitEvent, HANDLE &threadHandle);

#endif