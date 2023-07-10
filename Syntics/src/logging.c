
global b8 LOGGING = 1;
global b8 LOGGING_ALLOC = 1;
global void* logging_mutex = NULL;

#ifndef LINUX
void error_msg(const char* msg);
#endif

void init_logging()
{
    logging_mutex = CreateMutex(NULL, false, NULL);
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
    sprintf_s(buffer, 4094, "File: %s |-| Line: %d\n%s\n\n", file, line, msg);
    return buffer;
}

void _ERROR(const char* file, i32 line, const char* msg)
{
#ifdef LINUX
    fprintf(stderr, "%sERROR%s: File: %s: %d\nMessage: %s: %s%s%s\n", ANSI_COLOR_RED,
            ANSI_COLOR_RESET, file, line, msg, ANSI_COLOR_RED, strerror(errno),
            ANSI_COLOR_RESET);
#else

#endif

    char buffer[4096] = { 0 };
    time_t t = time(NULL);
    struct tm tmm = { 0 };
    localtime_s(&tmm, &t);
    sprintf_s(buffer, sizeof(buffer),
              "now: %02d-%02d-%d %02d:%02d:%02d\nFile: %s |-| Line: %d\n%s\n\n",
              tmm.tm_mday, tmm.tm_mon + 1, tmm.tm_year + 1900, tmm.tm_hour,
              tmm.tm_min, tmm.tm_sec, file, line, msg);

#ifndef CRASH_DEREF
#ifndef LINUX
    error_msg(buffer);
#endif
#endif
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
    OutputDebugString(buffer);
    printf(buffer);
    *(u32*)0 = 0;
}

global long volatile lock = 0;

void sy_print_text(char* text);

void sy_print(const char* format, ...)
{
    WaitForSingleObject(logging_mutex, INFINITE);

    va_list args;
    va_start(args, format);

    char buffer[512] = { 0 };

    vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

    // OutputDebugString(buffer);
    sy_print_text(buffer);

    va_end(args);

    ReleaseMutex(logging_mutex);
}
