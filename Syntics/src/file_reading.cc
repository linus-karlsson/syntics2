#include "file_reading.h"

b8 end_of_file(const File_Attrib& file)
{
    return file.current_pos >= file.size;
}

u32 read_line(File_Attrib* file, char* line, u32 max_size)
{
    u32 count = 0;
    while (!end_of_file(*file) && file->buffer[file->current_pos] != '\n' &&
           count < max_size - 1)
    {
        line[count++] = file->buffer[file->current_pos++];
    }
    if (file->buffer[file->current_pos] == '\n')
    {
        line[count++] = file->buffer[file->current_pos++];
    }
    line[count] = '\0';
    return count;
}

char* read_token(char* buffer, const char* delims)
{
    u32 len = 0;
    for (;;)
    {
        if(delims[len++] == '\0')
        {
            break;
        }
    }
    char* out = buffer;
    for (; *buffer != '\0'; buffer++)
    {
        for (u32 i = 0; i < len; i++)
        {
            if (*buffer == delims[i])
            {
                *buffer = '\0';
                return out;
            }
        }
    }
    return NULL;
}
