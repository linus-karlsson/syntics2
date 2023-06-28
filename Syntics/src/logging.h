#pragma once
#include <stdio.h>
#include "defines.h"

#define PR() sy_print("FILE: %s | LINE: %d\n", __FILE__, __LINE__)

#define SY_ERROR(msg) _ERROR(__FILE__, __LINE__, msg)

#define sy_prints(name, s, dt, ...)                                                    \
    presist f32 name = 0.0f;                                                        \
    name += dt;                                                                     \
    do                                                                              \
    {                                                                               \
        if (name >= s)                                                              \
        {                                                                           \
            sy_print(__VA_ARGS__);                                                     \
            name = 0.0f;                                                            \
        }                                                                           \
    } while (0)

#define sy_printss(name, dt, ...)                                                      \
    presist f32 name = 0.0f;                                                        \
    name += dt;                                                                     \
    do                                                                              \
    {                                                                               \
        if (name >= 0.5f)                                                           \
        {                                                                           \
            sy_print(__VA_ARGS__);                                                     \
            name = 0.0f;                                                            \
        }                                                                           \
    } while (0)

#define sy_printf32(v) sy_print("%f\n", (v))

#ifdef DEBUG
#define assert(ex)                                                                  \
    if (!(ex)) *(u32*)0 = 0
#else
#define assert(ex)
#endif

#define ASSERT(ex, text)                                                            \
    if (!(ex)) SY_ERROR(text)

#define val_to_str(buffer, ...) sprintf_s(buffer, sizeof((buffer)), __VA_ARGS__)

#define val_to_str_offset(buffer, offset, ...)                                      \
    sprintf_s((buffer) + (offset), sizeof((buffer)) - (offset), __VA_ARGS__)

#define str_to_val(buffer, ...) sscanf_s((buffer), __VA_ARGS__)

#define f32_to_str(buffer, num_digits, val)                                         \
    _gcvt_s(buffer, sizeof((buffer)), val, num_digits)

#define f32_to_str_offset(buffer, offset, num_digits, val)                          \
    _gcvt_s((buffer) + (offset), sizeof((buffer)) - (offset), val, num_digits)

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

void init_logging();
void set_log(b8 set_val);
b8 use_log(void);

void set_log_alloc(b8 set_val);
b8 use_log_alloc(void);
void sy_print_text(char* text);
void sy_print(const char* format, ...);

void _ERROR(const char* file, i32 line, const char* msg);
