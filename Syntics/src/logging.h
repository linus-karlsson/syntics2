#pragma once
#include <stdio.h>

namespace synt {

#define synt_LOG(...)                                                               \
    if (synt::use_log()) printf(__VA_ARGS__)

    void set_log(bool set_val);
    bool use_log();

    void ERROR(const char* msg);

} // namespace synt
