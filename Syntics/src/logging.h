#pragma once
#include <stdio.h>

namespace synt {

#define PR() synt_LOG("FILE: %s | LINE: %d\n", __FILE__, __LINE__)

#define synt_LOG(...)                                                          \
    if (synt::use_log()) printf(__VA_ARGS__)

#define synt_LOG_ALLOC(...)                                                    \
    if (synt::use_log_alloc()) printf(__VA_ARGS__)

#define ERROR(msg) _ERROR(__FILE__, __LINE__, msg)

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

void set_log(bool set_val);
bool use_log();

void set_log_alloc(bool set_val);
bool use_log_alloc();

void _ERROR(const char* file, int line, const char* msg);

} // namespace synt
