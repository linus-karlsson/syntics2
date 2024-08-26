#ifndef SY_UNIT_BUILD
#include "util.h"
#endif

f32 round_f32(f32 value)
{
    return (f32)((int)(value + (0.5f - (f32)(value < 0.0f))));
}

V2 round_v2(V2 v2)
{
    return v2f(round_f32(v2.x), round_f32(v2.y));
}

V4 v4_lerp(V4 v1, V4 v2, f32 t)
{
    return v4_add(v1, v4_s_multi(v4_sub(v2, v1), t));
}

V2 v2_lerp(V2 v1, V2 v2, f32 t)
{
    return v2_add(v1, v2_s_multi(v2_sub(v2, v1), t));
}
