
b8 end_of_file(const File_Attrib* file)
{
    return file->current_pos >= file->size;
}

u32 read_line(File_Attrib* file, char* line, u32 max_size, b8 remove_newline)
{
    u32 count = 0;
    while (!end_of_file(file) && file->buffer[file->current_pos] != '\n' &&
           count < max_size - 1)
    {
        line[count++] = file->buffer[file->current_pos++];
    }
    if (file->buffer[file->current_pos] == '\n')
    {
        if (!remove_newline)
        {
            line[count++] = file->buffer[file->current_pos];
        }
        file->current_pos++;
    }
    line[count] = '\0';
    return count;
}

char* read_token(char* buffer, u32 buffer_len, const char* delims)
{
    if (!buffer_len)
    {
        return NULL;
    }
    u32 len = 0;
    for (;;)
    {
        if (delims[len++] == '\0')
        {
            break;
        }
    }
    char* out = buffer;
    for (u32 i = 0; i < buffer_len; i++)
    {
        if (buffer[i] == '\0')
        {
            break;
        }
        for (u32 j = 0; j < len; j++)
        {
            if (*buffer == delims[j])
            {
                *buffer = '\0';
                return out;
            }
        }
    }
    return NULL;
}

u32 trim_string(char* string, u32 len)
{
    if (!len)
    {
        return 0;
    }
    char* start = NULL;
    i32 i = 0;
    for (; i < len; i++)
    {
        if (!start && string[i] != ' ')
        {
            start = string + i;
            break;
        }
    }
    char* end = NULL;
    for (i32 j = len - 1; j >= i; j--)
    {
        if (!end && string[j] != ' ' && string[j] != '\0')
        {
            end = string + j;
            i = (j - i) + 1; // plus 1 to get to the last empty spot
            break;
        }
    }
    if (i < len && i > 0)
    {
        if (string != start)
        {
            memcpy(string, start, i);
        }
        string[i] = '\0';
    }
    return (u32)i;
}
