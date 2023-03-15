#pragma once
#include <stdio.h>
#include "defines.h"

#define PR() synt_LOG("FILE: %s | LINE: %d\n", __FILE__, __LINE__)

#define SY_ERROR(msg) _ERROR(__FILE__, __LINE__, msg)

static char logging_buffer[1024] = { 0 };
#define synt_LOG_Term(...)                                                               \
    sprintf_s(logging_buffer, 1024, __VA_ARGS__);                                        \
    print_text(logging_buffer)

#define ASSERT(ex, text)                                                                 \
    if (!(ex)) SY_ERROR(text)

#define val_to_str(buffer, ...) sprintf_s(buffer, sizeof((buffer)), __VA_ARGS__)

#define val_to_str_offset(buffer, offset, ...)                                           \
    sprintf_s((buffer) + (offset), sizeof((buffer)) - (offset), __VA_ARGS__)

#define f32_to_str(buffer, num_digits, val)                                              \
    _gcvt_s(buffer, sizeof((buffer)), val, num_digits)

#define f32_to_str_offset(buffer, offset, num_digits, val)                               \
    _gcvt_s((buffer) + (offset), sizeof((buffer)) - (offset), val, num_digits)

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

void set_log(b8 set_val);
b8 use_log(void);

void set_log_alloc(b8 set_val);
b8 use_log_alloc(void);
void print_text(char* text);

void _ERROR(const char* file, i32 line, const char* msg);
