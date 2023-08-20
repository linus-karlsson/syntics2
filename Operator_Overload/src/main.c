#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "defines.h"
#include "file_reading.c"

const char* exe_file_path;
const char* output_file_path;
const char* format_source = "<source files>";
const char* format_output_file = "<output file name>";

void print_output_file_error()
{
    printf("ERROR: output file | %s | not found.\nUsage: %s %s %s\n",
           output_file_path, exe_file_path, format_source, format_output_file);
    exit(1);
}

void file_read(File_Attrib* file_attrib, const char* file_path,
               const char* operation)
{
    FILE* file = fopen(file_path, operation);

    if (file == NULL)
    {
        fprintf(stderr, "ERROR: File: %s\nMessage: %s\n", file_path,
                strerror(errno));
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    file_attrib->size = (u32)ftell(file);
    rewind(file);

    file_attrib->buffer = (u8*)malloc(file_attrib->size);

    if (fread(file_attrib->buffer, 1, file_attrib->size, file) !=
        file_attrib->size)
    {
        fprintf(stderr, "ERROR: File: %s\nMessage: %s\n", file_path,
                strerror(errno));
        exit(1);
    }
    fclose(file);
}

void file_write_entire(const char* file_path, const char* content, u32 size)
{
    FILE* file = fopen(file_path, "w");

    if (file == NULL)
    {
        fprintf(stderr, "ERROR: File: %s\nMessage: %s\n", file_path,
                strerror(errno));
        exit(1);
    }

    fwrite(content, 1, (size_t)size, file);
    fclose(file);
}

#define MAX_LINE_SIZE 1024 * 4

const char* function_name_heads[] = { "v3_", "v3_s_", "m4_", "m4_v3_" };

const char* function_name_tails[] = { "multi", "add", "sub", "s_multi" };

u32 parse_file(File_Attrib* file, char* buffer)
{
    const char* delims = " \n";
    const u32 delims_len = (u32)strlen(delims);
    const u32 max_line_size = MAX_LINE_SIZE;
    char line[MAX_LINE_SIZE] = { 0 };
    char* tokens[100] = { 0 };

    u32 buffer_size = 0;
    while (!end_of_file(file))
    {
        u32 line_len = line_read(file, line, max_line_size, false);

        u32 i = 0;
        while (i < line_len && (line[i] == ' ' || line[i] == '\r'))
        {
            i++;
        }
        if (i == line_len)
        {
            continue;
        }
        for (u32 j = 0; j < i; j++)
        {
            buffer[buffer_size++] = line[j];
        }
        const u32 token_count = token_read_all(line + i, line_len - i, delims,
                                               delims_len, tokens, 100);

        char* head = NULL;
        char* tail = NULL;
        u32 token_head_index = 0;
        for (i = 0; i < token_count; i++)
        {
            if (!strcmp(tokens[i], "v"))
            {
                head = "v3_";
                break;
            }
            else if (!strcmp(tokens[i], "vs"))
            {
                head = "v3_s_";
                break;
            }
            else if (!strcmp(tokens[i], "m"))
            {
                head = "m4_";
                break;
            }
            else if (!strcmp(tokens[i], "mv"))
            {
                head = "m4_v3_";
                break;
            }
        }
        if (head)
        {
            token_head_index = i++;
            if (!strcmp(tokens[i], "*"))
            {
                tail = "multi";
            }
            else if (!strcmp(tokens[i], "+"))
            {
                tail = "add";
            }
            else if (!strcmp(tokens[i], "-"))
            {
                tail = "sub";
            }
        }
        if (tail)
        {
            for (u32 j = 0; j < token_head_index - 1; j++)
            {
                u32 index = 0;
                while (tokens[j][index])
                {
                    buffer[buffer_size++] = tokens[j][index++];
                }
                buffer[buffer_size++] = ' ';
            }
            const u32 head_len = strlen(head);
            memcpy(buffer + buffer_size, head, head_len);
            buffer_size += head_len;
            const u32 tail_len = strlen(tail);
            memcpy(buffer + buffer_size, tail, tail_len);
            buffer_size += tail_len;
            buffer[buffer_size++] = '(';
            const u32 param0_len = strlen(tokens[token_head_index - 1]);
            memcpy(buffer + buffer_size, tokens[token_head_index - 1],
                   param0_len);
            buffer_size += param0_len;
            buffer[buffer_size++] = ',';
            buffer[buffer_size++] = ' ';
            u32 param1_len = strlen(tokens[token_head_index + 2]);
            if (token_head_index + 3 == token_count)
            {
                param1_len -= 1;
            }
            memcpy(buffer + buffer_size, tokens[token_head_index + 2],
                   param1_len);
            buffer_size += param1_len;
            buffer[buffer_size++] = ')';
            buffer[buffer_size++] = ';';
            buffer[buffer_size++] = '\n';
        }
        else
        {
            for (u32 j = 0; j < token_count - 1; j++)
            {
                u32 index = 0;
                while (tokens[j][index])
                {
                    buffer[buffer_size++] = tokens[j][index++];
                }
                buffer[buffer_size++] = ' ';
            }
            if(token_count)
            {
                u32 index = 0;
                while (tokens[token_count - 1][index])
                {
                    buffer[buffer_size++] = tokens[token_count - 1][index++];
                }
            }
            buffer[buffer_size++] = '\n';
        }
    }
    return buffer_size;
}

int main(int argc, char** argv)
{
    exe_file_path = argv[0];
    if (argc < 2)
    {
        printf("ERROR: format for source files wrong: %s %s %s\n",
               exe_file_path, format_source, format_output_file);
        exit(1);
    }
    else if (argc < 3)
    {
        output_file_path = "";
        print_output_file_error();
    }
    char* buffer = (char*)calloc(1024 * 1024 * 2, 1);

    File_Attrib file = { 0 };
    file_read(&file, argv[1], "r");

    u32 buffer_size = parse_file(&file, buffer);

    file_write_entire(argv[2], buffer, buffer_size);

    return 0;
}
