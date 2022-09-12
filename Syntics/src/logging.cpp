#include "logging.h"
#include <stdlib.h>

namespace synt {

    static bool LOGGING = 1;

    void set_log(bool set_val) { LOGGING = set_val; }
    bool use_log() { return LOGGING; }

    void ERROR(const char* msg)
    {
        puts(msg);
        exit(1);
    }

} // namespace synt

