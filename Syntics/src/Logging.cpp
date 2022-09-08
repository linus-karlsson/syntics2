#include "Logging.h"
#include <stdio.h>
#include <stdlib.h>

namespace synt {

    void LOG(const char* msg) { puts(msg); }
    void ERROR(const char* msg)
    {
        puts(msg);
        exit(1);
    }

} // namespace synt

