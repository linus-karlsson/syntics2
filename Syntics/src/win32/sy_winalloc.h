#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define PAGE_READWRITE 0x04

typedef void *LPVOID, *PVOID;
typedef unsigned long DWORD;

#define WINAPI __stdcall
LPVOID WINAPI VirtualAlloc(_In_opt_ LPVOID lpAddress, _In_ size_t dwSize,
                           _In_ DWORD flAllocationType, _In_ DWORD flProtect);
