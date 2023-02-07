#include "noise.h"

static int32 PERMUTATION[] = {
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

float sy_fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float sy_lerp(float a, float b, float t)
{
    return a + (t * (b - a));
}

static int32 SEED = 0;

// source nowl perlin.c
int32 sy_noise2(int32 x, int32 y)
{
    int32 tmp = PERMUTATION[(y + SEED) % 256];
    return PERMUTATION[(tmp + x) % 256];
}

float sy_smooth_inter(float a, float b, float t)
{
    return sy_lerp(a, b, t * t * (3 - 2 * t));
}

float sy_noise2d(float x, float y)
{
    int32 x_int = x;
    int32 y_int = y;
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    int32 s = sy_noise2(x_int, y_int);
    int32 t = sy_noise2(x_int + 1, y_int);
    int32 u = sy_noise2(x_int, y_int + 1);
    int32 v = sy_noise2(x_int + 1, y_int + 1);
    float low = sy_smooth_inter(s, t, x_frac);
    float high = sy_smooth_inter(u, v, x_frac);
    return sy_smooth_inter(low, high, y_frac);
}

float sy_value_noise2d(float x, float y, float freq, float gain, int32 oct)
{
    float amp = gain;
    float result = 0.0f;
    float max = 0.0f;

    for_range(i, oct)
    {
        max += 256.0f * amp;
        result += sy_noise2d(x * freq, y * freq) * amp;
        amp *= gain;
        freq *= 2.0f;
    }

    return result / max;
}
