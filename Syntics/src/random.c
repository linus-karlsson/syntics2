
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

