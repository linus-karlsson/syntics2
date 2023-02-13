#pragma once
#include "defines.h"

f32 sy_fade(f32 t);

f32 sy_normalize_f32(f32 value, f32 min, f32 max);

f32 sy_lerp(f32 a, f32 b, f32 t);

i32 sy_noise2(i32 x, i32 y);

f32 sy_smooth_inter(f32 a, f32 b, f32 t);

f32 sy_noise2d(f32 x, f32 y);

f32 sy_value_noise2d(f32 x, f32 y, f32 freq, f32 gain, i32 oct);
