#define FILE_READ_DATA (0x0001)      // file & pipe
#define FILE_LIST_DIRECTORY (0x0001) // directory

#define FILE_WRITE_DATA (0x0002) // file & pipe
#define FILE_ADD_FILE (0x0002)   // directory

#define FILE_APPEND_DATA (0x0004)          // file
#define FILE_ADD_SUBDIRECTORY (0x0004)     // directory
#define FILE_CREATE_PIPE_INSTANCE (0x0004) // named pipe

#define FILE_READ_EA (0x0008) // file & directory

#define FILE_WRITE_EA (0x0010) // file & directory

#define FILE_EXECUTE (0x0020)  // file
#define FILE_TRAVERSE (0x0020) // directory

#define FILE_DELETE_CHILD (0x0040) // directory

#define FILE_READ_ATTRIBUTES (0x0080) // all

#define FILE_WRITE_ATTRIBUTES (0x0100) // all

#define FILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF)

#define DELETE (0x00010000L)
#define READ_CONTROL (0x00020000L)
#define WRITE_DAC (0x00040000L)
#define WRITE_OWNER (0x00080000L)
#define SYNCHRONIZE (0x00100000L)

#define STANDARD_RIGHTS_REQUIRED (0x000F0000L)

#define STANDARD_RIGHTS_READ (READ_CONTROL)
#define STANDARD_RIGHTS_WRITE (READ_CONTROL)
#define STANDARD_RIGHTS_EXECUTE (READ_CONTROL)

#define STANDARD_RIGHTS_ALL (0x001F0000L)

#define SPECIFIC_RIGHTS_ALL (0x0000FFFFL)

#define FILE_GENERIC_READ                                                           \
    (STANDARD_RIGHTS_READ | FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA |  \
     SYNCHRONIZE)

#define FILE_GENERIC_WRITE                                                          \
    (STANDARD_RIGHTS_WRITE | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES |              \
     FILE_WRITE_EA | FILE_APPEND_DATA | SYNCHRONIZE)

#define FILE_GENERIC_EXECUTE                                                        \
    (STANDARD_RIGHTS_EXECUTE | FILE_READ_ATTRIBUTES | FILE_EXECUTE | SYNCHRONIZE)

#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2

#define GENERIC_READ (0x80000000L)
#define GENERIC_WRITE (0x40000000L)
#define GENERIC_EXECUTE (0x20000000L)
#define GENERIC_ALL (0x10000000L)

#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_SHARE_DELETE 0x00000004

#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5

#define WINAPI __stdcall

typedef void *LPVOID, *PVOID;
typedef PVOID HANDLE;
typedef unsigned long DWORD;
typedef const char *LPCSTR, *PCSTR;
typedef const wchar_t* LPCWSTR;

typedef __int64 LONGLONG;
typedef LONGLONG USN;

typedef long LONG;
typedef int INT;
typedef int BOOL;

typedef LONG* PLONG;

typedef __int64 LONG_PTR, *PLONG_PTR;
typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;

typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

typedef DWORD* LPDWORD;

typedef const void* LPCVOID;

#if defined(MIDL_PASS)
typedef struct _LARGE_INTEGER
{
    LONGLONG QuadPart;
} LARGE_INTEGER;
#else  // MIDL_PASS
typedef union _LARGE_INTEGER
{
    struct
    {
        DWORD LowPart;
        LONG HighPart;
    } DUMMYSTRUCTNAME;
    struct
    {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER;
#endif // MIDL_PASS

typedef LARGE_INTEGER* PLARGE_INTEGER;

typedef struct _SECURITY_ATTRIBUTES
{
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _OVERLAPPED
{
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union
    {
        struct
        {
            DWORD Offset;
            DWORD OffsetHigh;
        } DUMMYSTRUCTNAME;
        PVOID Pointer;
    } DUMMYUNIONNAME;

    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
HANDLE WINAPI CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess,
                          DWORD dwShareMode,
                          _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                          DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                          _In_opt_ HANDLE hTemplateFile);

HANDLE WINAPI CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                          DWORD dwShareMode,
                          _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                          DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                          _In_opt_ HANDLE hTemplateFile);

#ifdef UNICODE
#define CreateFile CreateFileW
#else
#define CreateFile CreateFileA
#endif // !UNICODE

BOOL WINAPI GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize);

BOOL WINAPI ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                     LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);

BOOL WINAPI CloseHandle(HANDLE hObject);

DWORD WINAPI SetFilePointer(HANDLE hFile, LONG lDistanceToMove,
                            PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);

BOOL WINAPI WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
                      LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
