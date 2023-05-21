// tells the preproccesor to not reorder things
#define fence _mm_mfence()
// tell the compiler to not reorder things
#define write_barrier                                                               \
    _WriteBarrier();                                                                \
    fence
#define read_barrier _ReadBarrier()

#define InterlockedIncrement _InterlockedIncrement
#define InterlockedExchange _InterlockedExchange
#define InterlockedCompareExchange _InterlockedCompareExchange
#define INFINITE 0xFFFFFFFF // Infinite timeout
#define WINAPI __stdcall
typedef long LONG;
typedef unsigned long DWORD;
typedef DWORD* LPDWORD;
typedef void* HANDLE;
typedef int BOOL;
typedef void *LPVOID, *PVOID;
typedef const char *LPCSTR, *PCSTR;
typedef const wchar_t* LPCWSTR;
typedef DWORD(WINAPI* PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

typedef struct _SECURITY_ATTRIBUTES
{
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

HANDLE WINAPI CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes,
                           size_t dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
                           LPVOID lpParameter, DWORD dwCreationFlags,
                           LPDWORD lpThreadId);

BOOL WINAPI CloseHandle(HANDLE hObject);

void WINAPI Sleep(DWORD dwMilliseconds);

HANDLE WINAPI CreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes,
                           BOOL bInitialOwner, LPCSTR lpName);

HANDLE WINAPI CreateMutexW(LPSECURITY_ATTRIBUTES lpMutexAttributes,
                           BOOL bInitialOwner, LPCWSTR lpName);
#ifdef UNICODE
#define CreateMutex CreateMutexW
#else
#define CreateMutex CreateMutexA
#endif // !UNICODE

DWORD WINAPI WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);

BOOL WINAPI ReleaseMutex(HANDLE hMutex);

HANDLE WINAPI CreateSemaphoreA(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
                               LONG lInitialCount, LONG lMaximumCount,
                               LPCSTR lpName);
#ifndef UNICODE
#define CreateSemaphore CreateSemaphoreA
#endif
