#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "defines.h"

#define assert(ex)                                                                  \
    if (!(ex)) *(u32*)0 = 0

#include "hash_table.h"
#include "hash_table.c"

typedef struct File_Attrib
{
    u8* buffer;
    u32 current_pos;
    u32 size;
} File_Attrib;

#include "file_reading.c"

HANDLE get_file_handle(LPCSTR file_path, DWORD operation, DWORD share_mode,
                       DWORD creation)
{
    HANDLE file = CreateFile(file_path, operation, share_mode, 0, creation, 0, 0);
    assert(file != INVALID_HANDLE_VALUE);
    return file;
}

u32 get_size(HANDLE file)
{
    LARGE_INTEGER file_size;
    assert(GetFileSizeEx(file, &file_size));
    return (u32)file_size.QuadPart;
}

void read_bytes(File_Attrib* file_attrib, HANDLE file)
{
    DWORD bytes_read;
    assert(ReadFile(file, file_attrib->buffer, file_attrib->size, &bytes_read, 0) &&
           file_attrib->size == bytes_read);

    CloseHandle(file);
}

void read_file_offset_arr(File_Attrib* file_attrib, const char* file_path,
                          const char* operation)
{
    HANDLE file =
        get_file_handle(file_path, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);

    file_attrib->size = get_size(file);
    file_attrib->buffer = (u8*)malloc(file_attrib->size);
    read_bytes(file_attrib, file);
}

void read_file(File_Attrib* file_attrib, const char* file_path,
               const char* operation)
{
    HANDLE file =
        get_file_handle(file_path, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);

    file_attrib->size = get_size(file);
    file_attrib->buffer = (u8*)malloc(file_attrib->size);
    file_attrib->current_pos = 0;
    read_bytes(file_attrib, file);
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


#define MAX_LINE_SIZE KILOBYTE(10)
void parse_file(File_Attrib* file)
{
#if 1
    const char* delims = " ";
    const u32 max_line_size = MAX_LINE_SIZE;
    char line[MAX_LINE_SIZE] = { 0 };
    char temp[MAX_LINE_SIZE] = { 0 };
    while (!end_of_file(file))
    {
        // TODO: nested scopes, fix that using recursion
        if (file->buffer[file->current_pos] == '{')
        {
            while (!end_of_file(file) && file->buffer[file->current_pos++] != '}')
                ;
            continue;
        }
        u32 line_len = read_line(file, line, max_line_size, true);
        line_len = trim_string(line, line_len);

        char* token = read_token(line, line_len, delims);
    }
#endif
}

void parse_directory(const char* directory, u32 directory_len)
{
    WIN32_FIND_DATA ffd = { 0 };
    char buffer[MAX_PATH];
    memcpy(buffer, directory, directory_len + 1);
    const u32 len_exclude_star = directory_len - 1;
    HANDLE file = FindFirstFile(buffer, &ffd);
    assert(file != INVALID_HANDLE_VALUE);
    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (!strcmp(ffd.cFileName, ".") || !strcmp(ffd.cFileName, ".."))
            {
                continue;
            }
            const u32 extension_len = (u32)strlen(ffd.cFileName);
            memcpy(buffer + len_exclude_star, ffd.cFileName, extension_len);
            u32 path_len = len_exclude_star + extension_len;
            buffer[path_len++] = '\\';
            buffer[path_len++] = '*';
            buffer[path_len] = '\0';
            parse_directory(buffer, path_len);
            printf("dir\n");
        }
        else
        {
            File_Attrib file_attrib = { 0 };
            const u32 extension_len = (u32)strlen(ffd.cFileName);
            memcpy(buffer + len_exclude_star, ffd.cFileName, extension_len);
            u32 path_len = len_exclude_star + extension_len;
            buffer[path_len] = '\0';

            read_file(&file_attrib, buffer, "");
            parse_file(&file_attrib);
            free(file_attrib.buffer);
            printf("Read: %s\n", ffd.cFileName);
        }
    } while (FindNextFile(file, &ffd) != 0);
}

int main()
{
    const char* dir = "Syntics\\src\\*";
    parse_directory(dir, (u32)strlen(dir));
    

    printf("Helo\n");
}
