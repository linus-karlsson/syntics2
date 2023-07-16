#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "defines.h"

#define assert(ex)                                                                  \
    if (!(ex)) *(u32*)0 = 0

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
    HANDLE file = CreateFile(file_path, operation, share_mode, 0, creation,
                             FILE_ATTRIBUTE_NORMAL, 0);
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

void iterate_scope(File_Attrib* file)
{
    while (!end_of_file(file) && file->buffer[file->current_pos++] != '}')
    {
        if (file->buffer[file->current_pos] == '{')
        {
            iterate_scope(file);
        }
    }
}

void iterate_if_scope(File_Attrib* file, Token* token, char* line, u32 max_line_size,
                      const char* what_if)
{
    while (!end_of_file(file))
    {
        u32 line_len = read_line(file, line, max_line_size, false);
        u32 trimmed_line_len = trim_string(line, line_len);
        *token = read_token(line, trimmed_line_len, " \r\n", 3);
        if (token->start)
        {
            if (!strcmp(token->start, what_if))
            {
                iterate_if_scope(file, token, line, max_line_size, what_if);
            }
            else if (!strcmp(token->start, "#endif"))
            {
                break;
            }
        }
    }
}

void iterate_perent_and_record(File_Attrib* file, char* line, u32* line_len)
{
    while (!end_of_file(file) && file->buffer[++file->current_pos] != ')')
    {
        line[(*line_len)++] = file->buffer[file->current_pos - 1];
        if (file->buffer[file->current_pos] == '(')
        {
            iterate_perent_and_record(file, line, line_len);
        }
    }
    line[(*line_len)++] = file->buffer[file->current_pos - 1];
}

b8 delim_exist(char* buffer, u32 buffer_len, char delim)
{
    for (u32 i = 0; i < buffer_len; i++)
    {
        if (buffer[i] == delim)
        {
            return true;
        }
    }
    return false;
}

u32 remove_windows_newline(char* buffer, u32 buffer_len)
{
    u32 last_new_line = 0;
    for (u32 i = 1; i < buffer_len; i++)
    {
        if (buffer[i] == '\n' && buffer[i - 1] == '\r')
        {
            last_new_line = i - 1;
            buffer[i - 1] = '\n';
            buffer[i] = ' ';
        }
    }
    return last_new_line;
}

#define MAX_LINE_SIZE KILOBYTE(10)
void parse_file(File_Attrib* file, const char* file_path, u32 file_path_len,
                const char* output_file_path)
{
    const char* delims = " \n";
    const u32 delims_len = (u32)strlen(delims);
    const u32 max_line_size = MAX_LINE_SIZE;
    char line[MAX_LINE_SIZE] = { 0 };

    { // File location for definitions
        sprintf_s(line, max_line_size, "///////// | %s | //////////////////////\n\n",
                  file_path);
        write_to_file(output_file_path, line);
    }

    while (!end_of_file(file))
    {
        if (file->buffer[file->current_pos] == '{')
        {
            iterate_scope(file);
            continue;
        }
        u32 line_len = read_line(file, line, max_line_size, true);
        u32 trimmed_line_len = trim_string(line, line_len);

        if (!trimmed_line_len || (line[0] == '/' && line[1] == '/') ||
            delim_exist(line, trimmed_line_len, ';'))
        {
            continue;
        }
        line[trimmed_line_len++] = '\n';
        line[trimmed_line_len] = '\0';

        Token token = read_token(line, trimmed_line_len, delims, delims_len);
        if (token.start)
        {
            if (!strcmp(token.start, "#include"))
            {
                continue;
            }
            if (!strcmp(token.start, "#define"))
            {
                reset_token(&token);
                char* temp = line;
                u32 define_line_len = trimmed_line_len;
                u32 total_len = trimmed_line_len;
                while (!end_of_file(file) &&
                       delim_exist(temp, define_line_len, '\\'))
                {
                    temp += define_line_len;
                    define_line_len = read_line(file, temp, max_line_size, false);
                    total_len += define_line_len;
                }
                remove_windows_newline(line, total_len);
                if (total_len != trimmed_line_len)
                {
                    line[total_len++] = '\n';
                    line[total_len] = '\0';
                }
                //write_to_file(output_file_path, line);
                continue;
            }
            if (!strcmp(token.start, "#if"))
            {
                if (token.start[token.delim_position + 1] == '0')
                {
                    iterate_if_scope(file, &token, line, max_line_size, "#if");
                }
                continue;
            }
            if (!strcmp(token.start, "#endif"))
            {
                continue;
            }
            if (!strcmp(token.start, "#ifdef"))
            {
                iterate_if_scope(file, &token, line, max_line_size, "#ifdef");
                continue;
            }
            if (!strcmp(token.start, "#ifndef"))
            {
                iterate_if_scope(file, &token, line, max_line_size, "#ifndef");
                continue;
            }
            if (!strcmp(token.start, "/*"))
            {
                file->current_pos -= line_len;
                reset_token(&token);
                while (!end_of_file(file))
                {
                    if (file->buffer[file->current_pos - 1] == '*' &&
                        file->buffer[file->current_pos] == '/')
                    {
                        read_line(file, line, max_line_size, true);
                        break;
                    }
                    file->current_pos++;
                }
                continue;
            }
            reset_token(&token);
            const u32 temp_store_position = file->current_pos;
            file->current_pos -= line_len + 1;
            u32 end = 0;
            char current_char = file->buffer[file->current_pos];
            b32 all_good = true;
            while (!end_of_file(file) && current_char != '(')
            {
                if (current_char == '{' || current_char == ';' ||
                    current_char == '=')
                {
                    all_good = false;
                    file->current_pos = temp_store_position;
                    break;
                }
                line[end++] = current_char = file->buffer[file->current_pos++];
            }
            if (all_good)
            {
                if (!end_of_file(file))
                {
                    if (file->buffer[file->current_pos] == ')')
                    {
                        line[end] = ')';
                    }
                    else
                    {
                        iterate_perent_and_record(file, line, &end);
                    }
                }
                if (!end_of_file(file))
                {
                    do
                    {
                        if (file->buffer[file->current_pos] == ';')
                        {
                            all_good = false;
                            break;
                        }
                        line[end] = file->buffer[file->current_pos++];

                    } while (!end_of_file(file) &&
                             file->buffer[file->current_pos] != '{' && end++);
                }
                if (all_good)
                {
                    u32 last = remove_windows_newline(line, end + 1);
                    if (last)
                    {
                        end = last;
                    }
                    end = trim_string(line, end);
                    line[end++] = ';';
                    line[end++] = '\n';
                    line[end++] = '\n';
                    line[end] = '\0';
                    write_to_file(output_file_path, line);
                }
            }
        }
    }
}

void parse_directory(const char* directory, u32 directory_len,
                     const char* output_file_name)
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
            parse_directory(buffer, path_len, output_file_name);
        }
        else
        {
            File_Attrib file_attrib = { 0 };
            const u32 extension_len = (u32)strlen(ffd.cFileName);
            memcpy(buffer + len_exclude_star, ffd.cFileName, extension_len);
            u32 path_len = len_exclude_star + extension_len;
            buffer[path_len] = '\0';

            read_file(&file_attrib, buffer, "");
            parse_file(&file_attrib, buffer, path_len, output_file_name);

            free(file_attrib.buffer);
        }
    } while (FindNextFile(file, &ffd) != 0);
}

int main()
{
    write_entire_file("syntics.h", "", 0);
    const char* dir = "Syntics\\src\\*";
    parse_directory(dir, (u32)strlen(dir), "syntics.h");
#if 0
    File_Attrib file_attrib = { 0 };
    read_file(&file_attrib, "Syntics\\src\\game.c", "");
    parse_file(&file_attrib, "Syntics\\src\\game.c",
               (u32)strlen("Syntics\\src\\game.c"), "syntics.h");
#endif
}
