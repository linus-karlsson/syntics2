#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

/*
void set_seed(void);
u32 rand_uint(u32 low, u32 high);
f32 rand_f32(f32 low, f32 high);
u32 rand_u32(f32 low, f32 high);
*/
u32 random_u32s(u32 seed);
u32 random_u32ss(u32 seed, u32 low, u32 high);
f32 random_f32s(u32 seed, f32 low, f32 high);

