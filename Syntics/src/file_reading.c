#include "file_reading.h"
#include "region_alloc.h"
#include <stdlib.h>
#include <Windows.h>

static HANDLE get_file_handle(LPCSTR file_path, DWORD operation, DWORD share_mode,
                              DWORD creation)
{
    HANDLE file = CreateFile(file_path, operation, share_mode, 0, creation, 0, 0);

    if (file == INVALID_HANDLE_VALUE)
    {
        OutputDebugString(file_path);
        SY_ERROR(file_path);
    }
    return file;
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
    HANDLE file =
        get_file_handle(file_path, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file, &file_size))
    {
        OutputDebugString("file size error");
        SY_ERROR("file size error");
    }
    file_attrib->size = (uint32_t)file_size.QuadPart;
#endif

    if (region)
    {
        file_attrib->buffer =
            region_mallocT(region, file_attrib->size, unsigned char);
        file_attrib->region_based = true;
    }
    else
    {
        file_attrib->buffer = (unsigned char*)malloc(file_attrib->size);
        file_attrib->region_based = false;
    }

#if LINUX
    if (fread(file_attrib.buffer, 1, file_attrib.size, file) != file_attrib.size)
    {
        OutputDebugString("Read file error");
        SY_ERROR(file_path);
    }
    fclose(file);
#else
    DWORD bytes_read;
    if (!ReadFile(file, file_attrib->buffer, file_attrib->size, &bytes_read, 0) ||
        file_attrib->size != bytes_read)
    {
        OutputDebugString("Read file error");
        SY_ERROR("");
    }
    CloseHandle(file);
#endif
}

void write_to_file(const char* file_path, const char* content)
{
    HANDLE file =
        get_file_handle(file_path, FILE_GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS);

    SetFilePointer(file, 0, NULL, FILE_END);

    DWORD bytes_written = 0;
    WriteFile(file, content, strlen(content), &bytes_written, 0);
    CloseHandle(file);
}

void write_entire_file(const char* file_path, const char* content)
{
    HANDLE file =
        get_file_handle(file_path, GENERIC_WRITE, FILE_SHARE_READ, CREATE_NEW);

    DWORD bytes_written = 0;
    WriteFile(file, content, strlen(content), &bytes_written, 0);
    CloseHandle(file);
}

void free_file(File_Attrib* file_attrib)
{
    free(file_attrib->buffer);
}
