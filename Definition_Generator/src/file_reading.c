
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

typedef struct Token
{
    char* start;
    u32 buffer_len;
    u32 delim_position;
    char delim_used;
} Token;

Token read_token(char* buffer, u32 buffer_len, const char* delims, u32 delims_len)
{
    Token result = { 0 };
    if (!buffer_len)
    {
        return result;
    }
    for (u32 i = 0; i < buffer_len; i++)
    {
        if (buffer[i] == '\0')
        {
            break;
        }
        for (u32 j = 0; j < delims_len; j++)
        {
            if (buffer[i] == delims[j])
            {
                buffer[i] = '\0';

                result.start = buffer;
                result.buffer_len = buffer_len;
                result.delim_position = i;
                result.delim_used = delims[j];
                return result;
            }
        }
    }
    return result;
}

void reset_token(Token* token)
{
    // TODO: bug 
    token->start[token->delim_position] = token->delim_used;
}

b8 read_next_token(Token* token, const char* delims, u32 delims_len)
{
    token->start[token->delim_position] = token->delim_used;
    token->start += token->delim_position + 1;
    u32 size = token->buffer_len - token->delim_position;
    for (u32 i = 0; i < size; i++)
    {
        if (token->start[i] == '\0')
        {
            break;
        }
        for (u32 j = 0; j < delims_len; j++)
        {
            if (token->start[i] == delims[j])
            {
                token->start[i] = '\0';

                token->buffer_len = size;
                token->delim_position = i;
                token->delim_used = delims[j];
                return true;
            }
        }
    }
    return false;
}

u32 trim_string(char* string, u32 len)
{
    if (!len)
    {
        return 0;
    }
    char* start = NULL;
    i32 i = 0;
    for (; i < (i32)len; i++)
    {
        if (!start && string[i] != ' ')
        {
            start = string + i;
            break;
        }
    }
    char* end = NULL;
    for (i32 j = (i32)len - 1; j >= i; j--)
    {
        if (!end && string[j] != ' ' && string[j] != '\r' && string[j] != '\0')
        {
            end = string + j;
            i = (j - i) + 1; // plus 1 to get to the last empty spot
            break;
        }
    }
    if (i < (i32)len && i > 0)
    {
        if (string != start)
        {
            memcpy(string, start, i);
        }
        string[i] = '\0';
    }
    return (u32)i;
}
