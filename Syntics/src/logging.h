#pragma once
#include <stdio.h>

namespace synt {

#define synt_LOG(...)                                                               \
    if (synt::use_log()) printf(__VA_ARGS__)

#define synt_LOG_ALLOC(...)                                                         \
    if (synt::use_log_alloc()) printf(__VA_ARGS__)

void set_log(bool set_val);
bool use_log();

void set_log_alloc(bool set_val);
bool use_log_alloc();

void ERROR(const char* msg);

} // namespace synt
