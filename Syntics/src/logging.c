#ifndef SY_UNIT_BUILD
#include "logging.h"
#include "platform.h"
#include "region_alloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#endif

global b8 LOGGING = 1;
global b8 LOGGING_ALLOC = 1;
global Mutex logging_mutex;
global u32 terminal_buffer_size_LOGGING = KILOBYTE(5);

global Terminal_Attrib LOGGING_TERM;

void logging_init(Region_Alloc* region)
{
    logging_mutex = syntics_platform_mutex_create();
    LOGGING_TERM.buffer = syntics_region_array(region, terminal_buffer_size_LOGGING, char);
    LOGGING_TERM.init = 1;
    LOGGING_TERM.auto_scroll = 1;
}

u32 terminal_get_buffer_size(void)
{
    return terminal_buffer_size_LOGGING;
}

Terminal_Attrib* terminal_get_ptr(void)
{
    return &LOGGING_TERM;
}

void set_log(b8 set_val)
{
    LOGGING = set_val;
}

b8 use_log(void)
{
    return LOGGING;
}

void set_log_alloc(b8 set_val)
{
    LOGGING_ALLOC = set_val;
}

b8 use_log_alloc(void)
{
    return LOGGING_ALLOC;
}

char* line_file_to_buffer(const char* file, i32 line, const char* msg)
{
    char* buffer = (char*)calloc(4094, 1);
    sysprintf(buffer, 4094, "File: %s |-| Line: %d\n%s\n\n", file, line, msg);
    return buffer;
}

void _ERROR(const char* file, i32 line, const char* msg)
{
    char buffer[4096] = { 0 };
    sysprintf(buffer, sizeof(buffer), "File: %s |-| Line: %d\n%s\n\n", file, line,
              msg);

    size_t len = strlen(buffer);
    size_t i = 0;
    for (; i < len; i++)
    {
        if (buffer[i] == '\n')
        {
            i += 80;
            break;
        }
    }
    for (; i < len; i += 80)
    {
        for (size_t s = i; s < len; s++)
        {
            if (buffer[s] == ' ')
            {
                buffer[s] = '\n';
                break;
            }
        }
    }
    printf("%s\n", buffer);
    *(u32*)0 = 0;
}

global long volatile lock = 0;

void sy_print_text(Terminal_Attrib* term, char* text);

void sy_print(const char* format, ...)
{
    syntics_platform_mutex_lock(&logging_mutex);

    va_list args;
    va_start(args, format);

    char buffer[512] = { 0 };

    vsnprintf(buffer, sizeof(buffer), format, args);

    sy_print_text(terminal_get_ptr(), buffer);

    va_end(args);

    syntics_platform_mutex_unlock(&logging_mutex);
}
