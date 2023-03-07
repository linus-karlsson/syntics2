#pragma once
#include <stdio.h>
#include "defines.h"

#define PR() synt_LOG("FILE: %s | LINE: %d\n", __FILE__, __LINE__)

static char logging_buffer[1024] = { 0 };
#define synt_LOG_Term(...)                                                          \
    do                                                                              \
    {                                                                               \
        if (use_log())                                                              \
        {                                                                           \
            sprintf(logging_buffer, __VA_ARGS__);                                   \
            print_text(logging_buffer);                                             \
        }                                                                           \
    } while (0)

#define synt_LOG(...)                                                               \
    do                                                                              \
    {                                                                               \
        if (use_log())                                                              \
        {                                                                           \
            printf("%s[INFO]:%s ", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);             \
            printf(__VA_ARGS__);                                                    \
        }                                                                           \
    } while (0)

#define synt_LOG_ALLOC(...)                                                         \
    if (use_log_alloc()) printf(__VA_ARGS__)

#define SY_ERROR(msg) _ERROR(__FILE__, __LINE__, msg)

#define ASSERT(ex, text)                                                            \
    if (!(ex)) SY_ERROR(text)

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

void set_log(b8 set_val);
b8 use_log();

void set_log_alloc(b8 set_val);
b8 use_log_alloc();
void print_text(char* text);

void _ERROR(const char* file, i32 line, const char* msg);

