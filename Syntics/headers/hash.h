#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

u64 hash_murmur(const char* key, u64 len, u64 seed),
