typedef struct File_Attrib
{
    u8* buffer;
    u32 current_pos;
    u32 size;
} File_Attrib;

typedef struct Token
{
    char* start;
    u32 buffer_len;
    u32 delim_position;
    char delim_used;
} Token;

b8 end_of_file(const File_Attrib* file)
{
    return file->current_pos >= file->size;
}

b8 is_delim(char character, char* delims, u32 delim_len)
{
    for (u32 i = 0; i < delim_len; i++)
    {
        if (delims[i] == character)
        {
            return true;
        }
    }
    return false;
}

u32 buffer_read(File_Attrib* file, char* buffer, u32 buffer_size, char* delims,
                u32 delim_len, b8 remove_character)
{
    u32 count = 0;
    while (!end_of_file(file) &&
           !is_delim(file->buffer[file->current_pos], delims, delim_len) &&
           count < buffer_size - 1)
    {
        buffer[count++] = file->buffer[file->current_pos++];
    }
    if (is_delim(file->buffer[file->current_pos], delims, delim_len))
    {
        if (!remove_character)
        {
            buffer[count++] = file->buffer[file->current_pos];
        }
        file->current_pos++;
    }
    buffer[count] = '\0';
    return count;
}

u32 line_read(File_Attrib* file, char* line, u32 line_size, b8 remove_newline)
{
    return buffer_read(file, line, line_size, "\n", 1, remove_newline);
}

u32 statement_read(File_Attrib* file, char* buffer, u32 buffer_size,
                   b8 remove_newline)
{
    u32 res = buffer_read(file, buffer, buffer_size, "{;", 2, false);
    while (!end_of_file(file) && file->buffer[file->current_pos] != '\n')
    {
        buffer[res++] = file->buffer[file->current_pos++];
    }
    if (!remove_newline && file->buffer[file->current_pos] == '\n')
    {
        buffer[res++] = file->buffer[file->current_pos++];
    }
    buffer[res] = '\0';
    return res;
}

Token token_read(char* buffer, u32 buffer_len, const char* delims,
                 u32 delims_len)
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

u32 token_read_all(char* buffer, u32 buffer_len, const char* delims,
                   u32 delims_len, Token* tokens, u32 token_count)
{
    u32 count = 0;
    if (!buffer_len)
    {
        return count;
    }
    u32 last_token_end_position = 0;
    u32 current_token_len = 0;
    for (u32 i = 0; i < buffer_len; i++)
    {
        b32 should_increase = true;
        if (buffer[i] == '\0')
        {
            break;
        }
        for (u32 j = 0; j < delims_len; j++)
        {
            if (buffer[i] == delims[j])
            {
                tokens[count].start = buffer + last_token_end_position;
                tokens[count].buffer_len = buffer_len;
                tokens[count].delim_position = current_token_len;
                tokens[count++].delim_used = delims[j];
                current_token_len = 0;

                buffer[i] = '\0';
                last_token_end_position = i + 1;
#if 0
                b8 found = true;
                while (found && i < buffer_len)
                {
                    found = false;
                    for (u32 k = 0; k < delims_len; k++)
                    {
                        if (buffer[i] == delims[j])
                        {
                            found = true;
                        }
                    }
                    if (found)
                    {
                        i++;
                    }
                }
                found = true;
#endif
                should_increase = false;
                break;
            }
        }
        if (should_increase)
        {
            current_token_len++;
        }
    }
    return count;
}

void token_reset(Token* token)
{
    // TODO: bug
    token->start[token->delim_position] = token->delim_used;
}

b8 token_read_next(Token* token, const char* delims, u32 delims_len)
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

u32 string_trim(char* string, u32 len)
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
