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

    u32 buffer_size = 0;
    while (!end_of_file(file))
    {
        u32 line_len = statement_read(file, line, max_line_size, false);

        // INFO: if the line buffer is too long it could not be correct so
        // skipping it
        if (line_len < 256)
        {
            // To keep the formatting correct
            i32 i = 0;
            while (i < line_len && (line[i] == ' ' || line[i] == '\r'))
            {
                i++;
            }
            for (u32 j = 0; j < i; j++)
            {
                buffer[buffer_size++] = line[j];
            }
            if (i == line_len)
            {
                continue;
            }

            Token tokens[1024] = { 0 };
            const u32 token_count = token_read_all(
                line + i, line_len - i, delims, delims_len, tokens, 100);

            u32 temp_size = 0;
            char temp[MAX_LINE_SIZE] = { 0 };
            u32 token_head_index = 0;
            b32 found = false;
            for (i = token_count - 1; i >= 0; i--)
            {
                char* head = NULL;
                char* tail = NULL;
                if (!strcmp(tokens[i].start, "*"))
                {
                    tail = "multi(";
                }
                else if (!strcmp(tokens[i].start, "+"))
                {
                    tail = "add(";
                }
                else if (!strcmp(tokens[i].start, "-"))
                {
                    tail = "sub(";
                }
                if (tail)
                {
                    token_head_index = i - 1;
                    if (!strcmp(tokens[token_head_index].start, "v"))
                    {
                        head = "v3_";
                    }
                    else if (!strcmp(tokens[token_head_index].start, "vs"))
                    {
                        head = "v3_s_";
                    }
                    else if (!strcmp(tokens[token_head_index].start, "m"))
                    {
                        head = "m4_";
                    }
                    else if (!strcmp(tokens[token_head_index].start, "mv"))
                    {
                        head = "m4_v3_";
                    }
                }
                if (head)
                {
                    const u32 head_len = strlen(head);
                    const u32 tail_len = strlen(tail);
                    const u32 param0_len =
                        tokens[token_head_index - 1].delim_position;
                    const u32 total_size = head_len + tail_len + param0_len;
                    const u32 param1_len = temp_size;

                    memcpy(temp + total_size + 2, temp, temp_size);
                    temp[total_size + 1] = ' ';
                    temp[total_size] = ',';
                    temp_size += total_size + 2;
                    memcpy(temp + head_len + tail_len,
                           tokens[token_head_index - 1].start, param0_len);
                    memcpy(temp + head_len, tail, tail_len);
                    memcpy(temp, head, head_len);
                    for (u32 j = 0; j < temp_size; j++)
                    {
                        if (temp[j] == ';')
                        {
                            temp[j] = ')';
                            temp[j + 1] = ';';
                            temp[j + 2] = '\n';
                            temp[j + 3] = '\0';
                            temp_size = j + 3;
                            found = true;
                            break;
                        }
                    }
                    i -= 2;
                }
                else
                {
                    const u32 token_len = tokens[i].delim_position + 1;
                    memcpy(temp + token_len, temp, temp_size);
                    memcpy(temp, tokens[i].start, token_len);
                    temp[tokens[i].delim_position] = tokens[i].delim_used;
                    temp_size += token_len;
                }
            }
            memcpy(buffer + buffer_size, temp, temp_size);
            buffer_size += temp_size;
        }
        else
        {
            memcpy(buffer + buffer_size, line, line_len);
            buffer_size += line_len;
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
