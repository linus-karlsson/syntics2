#include "logging.h"
#include "file_reading.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

static b8 LOGGING = 1;
static b8 LOGGING_ALLOC = 1;

#ifndef LINUX
void error_msg(const char* msg);
#endif

void set_log(b8 set_val)
{
    LOGGING = set_val;
}
b8 use_log()
{
    return LOGGING;
}

void set_log_alloc(b8 set_val)
{
    LOGGING_ALLOC = set_val;
}
b8 use_log_alloc()
{
    return LOGGING_ALLOC;
}

void _ERROR(const char* file, i32 line, const char* msg)
{
#ifdef LINUX
    fprintf(stderr, "%sERROR%s: File: %s: %d\nMessage: %s: %s%s%s\n", ANSI_COLOR_RED,
            ANSI_COLOR_RESET, file, line, msg, ANSI_COLOR_RED, strerror(errno),
            ANSI_COLOR_RESET);
#else

#endif

    char buffer[4096] = {};
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(buffer, "now: %02d-%02d-%d %02d:%02d:%02d\n%s\n\n", tm.tm_mday,
            tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, msg);

#ifndef LINUX
    error_msg(buffer);
#endif
    size_t len = strlen(buffer);
    i32 i = 0;
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
    write_to_file("error_logging.txt", buffer);
    exit(1);
}

