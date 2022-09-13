#pragma once

#include "vectors.h"

namespace synt {
    typedef struct Ray
    {
        Vec3 direction;
        Point3f origin;
    } Ray;
} // namespace synt
