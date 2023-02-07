#pragma once
#include "defines.h"

float sy_fade(float t);

float sy_lerp(float a, float b, float t);

int32 sy_noise2(int32 x, int32 y);

float sy_smooth_inter(float a, float b, float t);

float sy_noise2d(float x, float y);

float sy_value_noise2d(float x, float y, float freq, float gain, int32 oct);
