#include "file_reading.h"
#include "region_alloc.h"
#include <stdlib.h>
#include <Windows.h>

File_Attrib::File_Attrib() : buffer(0), size(0), region_based(0)
{
}
File_Attrib::~File_Attrib()
{
    if (!region_based)
    {
        if (buffer) free(buffer);
    }
}

File_Attrib read_file(Region_Alloc* region, const char* file_path,
                      const char* operation)
{
    File_Attrib file_attrib;
#ifdef LINUX
    FILE* file = fopen(file_path, operation);

    if (file == NULL) SY_ERROR(file_path);

    fseek(file, 0, SEEK_END);
    file_attrib.size = (uint32)ftell(file);
    rewind(file);
#else
    HANDLE file = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, 0,
                              OPEN_EXISTING, 0, 0);
    if (file == INVALID_HANDLE_VALUE)
    {
        OutputDebugString(file_path);
        SY_ERROR(file_path);
    }
    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file, &file_size))
    {
        OutputDebugString("file size error");
        SY_ERROR("file size error");
    }
    file_attrib.size = (uint32_t)file_size.QuadPart;
#endif

    if (region)
    {
        file_attrib.buffer = region_mallocT(region, file_attrib.size, unsigned char);
        file_attrib.region_based = true;
    }
    else
    {
        file_attrib.buffer = (unsigned char*)malloc(file_attrib.size);
        file_attrib.region_based = false;
    }

#ifdef LINUX
    if (fread(file_attrib.buffer, 1, file_attrib.size, file) != file_attrib.size)
    {
        SY_ERROR(file_path);
    }
    fclose(file);
#else
    DWORD bytes_read;
    if (!ReadFile(file, file_attrib.buffer, file_attrib.size, &bytes_read, 0) ||
        file_attrib.size != bytes_read)
    {
        OutputDebugString("Read file error");
        SY_ERROR("");
    }

#endif

    return file_attrib;
}

