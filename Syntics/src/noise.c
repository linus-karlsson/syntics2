
static const i32 PERMUTATION[] = {
    151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225,
    140, 36,  103, 30,  69,  142, 8,   99,  37,  240, 21,  10,  23,  190, 6,   148,
    247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203, 117, 35,  11,  32,
    57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136, 171, 168, 68,  175,
    74,  165, 71,  134, 139, 48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122,
    60,  211, 133, 230, 220, 105, 92,  41,  55,  46,  245, 40,  244, 102, 143, 54,
    65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169,
    200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,
    52,  217, 226, 250, 124, 123, 5,   202, 38,  147, 118, 126, 255, 82,  85,  212,
    207, 206, 59,  227, 47,  16,  58,  17,  182, 189, 28,  42,  223, 183, 170, 213,
    119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,
    129, 22,  39,  253, 19,  98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104,
    218, 246, 97,  228, 251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241,
    81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157,
    184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,
    222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,  156, 180
};

f32 sy_normalize_f32(f32 value, f32 min, f32 max)
{
    return (value - min) / (max - min);
}

// Ken perlin
f32 sy_fade(f32 t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

f32 sy_lerp(f32 a, f32 b, f32 t)
{
    return a + (t * (b - a));
}

static i32 SEED = 0;

// source nowl perlin.c
i32 sy_noise2(i32 x, i32 y)
{
    i32 tmp = PERMUTATION[(y + SEED) % 256];
    return PERMUTATION[(tmp + x) % 256];
}

// https://en.wikipedia.org/wiki/Smoothstep
//
f32 sy_smooth_inter(f32 a, f32 b, f32 t)
{
    return sy_lerp(a, b, t * t * (3 - 2 * t));
}

f32 sy_noise2d(f32 x, f32 y)
{
    i32 x_int = (i32)x;
    i32 y_int = (i32)y;
    f32 x_frac = x - x_int;
    f32 y_frac = y - y_int;
    i32 s = sy_noise2(x_int, y_int);
    i32 t = sy_noise2(x_int + 1, y_int);
    i32 u = sy_noise2(x_int, y_int + 1);
    i32 v = sy_noise2(x_int + 1, y_int + 1);
    f32 low = sy_smooth_inter((f32)s, (f32)t, x_frac);
    f32 high = sy_smooth_inter((f32)u, (f32)v, x_frac);
    return sy_smooth_inter(low, high, y_frac);
}

f32 sy_value_noise2d(f32 x, f32 y, f32 freq, f32 gain, i32 oct)
{
    f32 amp = gain;
    f32 result = 0.0f;
    f32 max = 0.0f;

    for (int i = 0; i < oct; i++)
    {
        max += 256.0f * amp;
        result += sy_noise2d(x * freq, y * freq) * amp;
        amp *= gain;
        freq *= 2.0f;
    }

    return result / max;
}
