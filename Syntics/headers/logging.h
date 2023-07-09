#pragma once

#define PR() sy_print("FILE: %s | LINE: %d\n", __FILE__, __LINE__)

#define SY_ERROR(msg) _ERROR(__FILE__, __LINE__, msg)

#define sy_prints(name, s, dt, ...)                                                 \
    presist f32 name = 0.0f;                                                        \
    name += dt;                                                                     \
    do                                                                              \
    {                                                                               \
        if (name >= s)                                                              \
        {                                                                           \
            sy_print(__VA_ARGS__);                                                  \
            name = 0.0f;                                                            \
        }                                                                           \
    } while (0)

#define sy_printss(name, dt, ...)                                                   \
    presist f32 name = 0.0f;                                                        \
    name += dt;                                                                     \
    do                                                                              \
    {                                                                               \
        if (name >= 0.5f)                                                           \
        {                                                                           \
            sy_print(__VA_ARGS__);                                                  \
            name = 0.0f;                                                            \
        }                                                                           \
    } while (0)

#define sy_printf32(v) sy_print("%f\n", (v))

#define assert(ex)                                                                  \
    do                                                                              \
    {                                                                               \
        if (!(ex))                                                                  \
        {                                                                           \
            OutputDebugString(line_file_to_buffer(__FILE__, __LINE__, "ASSERT!!")); \
            *(u32*)0 = 0;                                                           \
        }                                                                           \
    } while (0)

#if 0
#ifdef DEBUG
#define assert(ex)                                                                  \
    if (!(ex)) *(u32*)0 = 0
#else
#define assert(ex)
#endif
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
