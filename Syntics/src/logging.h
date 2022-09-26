#pragma once
#include <stdio.h>

namespace synt {

#define synt_LOG(...)                                                          \
    if (synt::use_log()) printf(__VA_ARGS__)

#define synt_LOG_ALLOC(...)                                                    \
    if (synt::use_log_alloc()) printf(__VA_ARGS__)

#define ERROR(msg) _ERROR(__FILE__, __LINE__, msg)

void set_log(bool set_val);
bool use_log();

void set_log_alloc(bool set_val);
bool use_log_alloc();

void _ERROR(const char* file, int line, const char* msg);

} // namespace synt
