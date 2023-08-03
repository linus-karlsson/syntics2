
void set_seed(void)
{
    srand((unsigned int)time(NULL));
}

u32 random_uint(u32 low, u32 high)
{
    return ((u32)rand() % (high - low + 1) + low);
}

f32 random_f32(f32 low, f32 high)
{
    return (float)((f32)rand() / (RAND_MAX / (high - low)) + low);
}

#define RANDOM_MAX 0x7FFFFFFF

u32 random_u32s(u32 seed)
{
    seed = (seed << 13) ^ seed;
    return ((seed * (seed * seed * 15731 + 789221) + 1376312589) & RANDOM_MAX);
}

u32 random_u32ss(u32 seed, u32 low, u32 high)
{
    return (random_u32s(seed) / (RANDOM_MAX / (high + 1 - low))) + low;
}

f32 random_f32s(u32 seed, f32 low, f32 high)
{
    return ((f32)random_u32s(seed) / ((f32)RANDOM_MAX / (high - low))) + low;
}

