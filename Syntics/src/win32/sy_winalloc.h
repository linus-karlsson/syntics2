#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define PAGE_READWRITE 0x04

typedef void *LPVOID, *PVOID;
typedef unsigned long DWORD;

#define WINAPI __stdcall
LPVOID WINAPI VirtualAlloc(LPVOID lpAddress, size_t dwSize, DWORD flAllocationType,
                           DWORD flProtect);
