#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

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

Token token_read(char* buffer, u32 buffer_len, const char* delims, u32 delims_len);
b8    end_of_file(const File_Attrib* file);
b8    is_delim(char character, char* delims, u32 delim_len);
u32   buffer_read(File_Attrib* file, char* buffer, u32 buffer_size, char* delims, u32 delim_len, b8 remove_character);
u32   line_read(File_Attrib* file, char* line, u32 line_size, b8 remove_newline);
u32   statement_read(File_Attrib* file, char* buffer, u32 buffer_size, b8 remove_newline);
u32   token_read_all(char* buffer, u32 buffer_len, const char* delims, u32 delims_len, Token* tokens, u32 token_count);
void  token_reset(Token* token);
b8    token_read_next(Token* token, const char* delims, u32 delims_len);
u32   string_trim(char* string, u32 len);
