#include "random.h"
#include <time.h>
#include <stdlib.h>

namespace synt {

void set_seed()
{
    srand(time(NULL));
}

uint32 rand_uint(uint32 low, uint32 high)
{
    return (rand() % (high - low + 1) + low);
}

float rand_f32(float low, float high)
{
    return (float)(rand() / (RAND_MAX / (high - low)) + low);
}

} // namespace synt
