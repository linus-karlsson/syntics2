#include "logging.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static bool LOGGING = 1;
static bool LOGGING_ALLOC = 1;

void set_log(bool set_val)
{
    LOGGING = set_val;
}
bool use_log()
{
    return LOGGING;
}

void set_log_alloc(bool set_val)
{
    LOGGING_ALLOC = set_val;
}
bool use_log_alloc()
{
    return LOGGING_ALLOC;
}

void _ERROR(const char* file, int line, const char* msg)
{
    fprintf(stderr, "%sERROR%s: File: %s: %d\nMessage: %s: %s%s%s\n", ANSI_COLOR_RED,
            ANSI_COLOR_RESET, file, line, msg, ANSI_COLOR_RED, strerror(errno),
            ANSI_COLOR_RESET);
    exit(1);
}

