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

// NOTE: ALL this is for my vim config
#ifdef CRASH_DEREF
#ifdef LINUX
#define crash(str)                                                                     \
    do                                                                              \
    {                                                                               \
        const char* buffer_ASSERT =                                                 \
            line_file_to_buffer(__FILE__, __LINE__, "ASSERT!!");                    \
        printf("%s\n%s\n", str, buffer_ASSERT);                                              \
        *(u32*)0 = 0;                                                               \
    } while (0)
#else
#define crash() *(u32*)0 = 0
//#define crash() asm("int $3")
#endif
#else
#define crash() SY_ERROR("ASSERT");
#endif

#if 1
#if 1
#define assert(ex)                                                                  \
    if (!(ex)) crash(#ex)
#else
#define assert(ex) ASSERT(ex, "")
#endif
#endif

#define assert_static(ex, msg) _Static_assert(ex, msg)

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

#define val_to_str(buffer, ...) sysprintf(buffer, sizeof((buffer)), __VA_ARGS__)

#define val_to_str_offset(buffer, offset, ...)                                      \
    sysprintf((buffer) + (offset), sizeof((buffer)) - (offset), __VA_ARGS__)

#define str_to_val(buffer, ...) syscanf((buffer), __VA_ARGS__)

#define f32_to_str(buffer, num_digits, val)                                         \
    sy_gcvt(buffer, sizeof((buffer)), val, num_digits)

#define f32_to_str_offset(buffer, offset, num_digits, val)                          \
    sy_gcvt((buffer) + (offset), sizeof((buffer)) - (offset), val, num_digits)

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

typedef struct Terminal_Attrib
{
    V2 dimensions;
    VkRect2D scissor;
    u32 index_offset;
    u32 num_indices;
    u32 presist_offset_x;
    u32 presist_offset_y;

    char* buffer;

    b8 init;
    b8 auto_scroll;
    b8 presist_hold;
} Terminal_Attrib;

void logging_init(Region_Alloc* region);
Terminal_Attrib* terminal_ptr_get();
char* line_file_to_buffer(const char* file, i32 line, const char* msg);
void _ERROR(const char* file, i32 line, const char* msg);
void sy_print(const char* format, ...);

