#include "logging.h"
#include <stdio.h>
#include <stdlib.h>

namespace synt {

    void ERROR(const char* msg)
    {
        puts(msg);
        exit(1);
    }

} // namespace synt

