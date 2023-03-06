#include "random.h"
#include <time.h>
#include <stdlib.h>

void set_seed()
{
    srand((unsigned int)time(NULL));
}

u32 rand_uint(u32 low, u32 high)
{
    return (rand() % (high - low + 1) + low);
}

f32 rand_f32(f32 low, f32 high)
{
    return (float)(rand() / (RAND_MAX / (high - low)) + low);
}

