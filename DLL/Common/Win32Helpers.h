#pragma once

inline void SAFE_CLOSE_HANDLE(HANDLE& a)
{
    if (a != NULL)
    {
        CloseHandle(a);
        a = NULL;
    }
}

template <typename T>
inline void SAFE_RELEASE(T*& a)
{
    if (a != NULL) 
    {
        a->Release();
        a = NULL;
    }
}

template <typename S>
inline void SAFE_CLOSE_SOCKET(S& a)
{
    if (a != INVALID_SOCKET)
    {
        closesocket(a);
        a = INVALID_SOCKET;
    }
}

inline void KillThread(HANDLE exitEvent, HANDLE &threadHandle)
{
    if (exitEvent)
        SetEvent(exitEvent);

    if (threadHandle != nullptr)
    {
        DWORD result = 0;
        if (GetExitCodeThread(threadHandle, &result) != 0
            && result == STILL_ACTIVE)
        {
            WaitForSingleObject(threadHandle, INFINITE);
        }
        SAFE_CLOSE_HANDLE(threadHandle);
    }
}
