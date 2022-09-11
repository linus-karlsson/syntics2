#pragma once

namespace synt {

    static bool LOGGING = 1;

#define synt_LOG(...)                                                               \
    if (synt::LOGGING) printf(__VA_ARGS__);

    void ERROR(const char* msg);

} // namespace synt
