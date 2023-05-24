#include "file_reading.h"
#include "region_alloc.h"
#include <stdlib.h>
#if 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include "win32\sy_winfile.h"
#endif

static HANDLE get_file_handle(LPCSTR file_path, DWORD operation, DWORD share_mode,
                              DWORD creation)
{
    HANDLE file = CreateFile(file_path, operation, share_mode, 0, creation, 0, 0);

    if (file == INVALID_HANDLE_VALUE)
    {
        SY_ERROR(file_path);
    }
    return file;
}

static HANDLE get_size(File_Attrib* file_attrib, const char* file_path)
{
    HANDLE file =
        get_file_handle(file_path, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file, &file_size))
    {
        SY_ERROR("file size error");
    }
    file_attrib->size = (u32)file_size.QuadPart;
    return file;
}

void read_bytes(File_Attrib* file_attrib, HANDLE file)
{
    DWORD bytes_read;
    if (!ReadFile(file, file_attrib->buffer, file_attrib->size, &bytes_read, 0) ||
        file_attrib->size != bytes_read)
    {
        SY_ERROR("");
    }
    CloseHandle(file);
}

void read_file_offset_arr(File_Attrib* file_attrib, Region_Alloc* region,
                          const char* file_path, const char* operation)
{
    HANDLE file = get_size(file_attrib, file_path);
    if (region)
    {
        region->currentPos += file_attrib->size + sizeof(Array_Head);
        file_attrib->buffer = region_mallocT(region, file_attrib->size, unsigned char);
        region->currentPos -= file_attrib->size + sizeof(Array_Head);
    }
    else
    {
        file_attrib->buffer = (unsigned char*)malloc(file_attrib->size);
    }
    read_bytes(file_attrib, file);
}

void read_file(File_Attrib* file_attrib, Region_Alloc* region, const char* file_path,
               const char* operation)
{
#if LINUX
    FILE* file = fopen(file_path, operation);

    if (file == NULL) SY_ERROR(file_path);

    fseek(file, 0, SEEK_END);
    file_attrib.size = (uint32)ftell(file);
    rewind(file);
#else
    HANDLE file = get_size(file_attrib, file_path);
#endif

    if (region)
    {
        file_attrib->buffer = region_mallocT(region, file_attrib->size, unsigned char);
    }
    else
    {
        file_attrib->buffer = (unsigned char*)malloc(file_attrib->size);
    }

#if LINUX
    if (fread(file_attrib.buffer, 1, file_attrib.size, file) != file_attrib.size)
    {
        OutputDebugString("Read file error");
        SY_ERROR(file_path);
    }
    fclose(file);
#else
    read_bytes(file_attrib, file);
#endif
}

void write_to_file(const char* file_path, const char* content)
{
    HANDLE file =
        get_file_handle(file_path, FILE_GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS);

    SetFilePointer(file, 0, NULL, FILE_END);

    DWORD bytes_written = 0;
    WriteFile(file, content, (DWORD)strlen(content), &bytes_written, 0);
    CloseHandle(file);
}

void write_entire_file(const char* file_path, const char* content, u32 size)
{
    HANDLE file =
        get_file_handle(file_path, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);

    DWORD bytes_written = 0;
    WriteFile(file, content, (DWORD)size, &bytes_written, 0);
    CloseHandle(file);
}

void free_file(File_Attrib* file_attrib)
{
    free(file_attrib->buffer);
}
