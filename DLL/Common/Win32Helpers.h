#pragma once

inline void SAFE_CLOSE_HANDLE(HANDLE& a)
{
    if (a != nullptr)
    {
        CloseHandle(a);
        a = nullptr;
    }
}

template <typename T>
inline void SAFE_RELEASE(T*& a)
{
    if (a != nullptr) 
    {
        a->Release();
        a = nullptr;
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
