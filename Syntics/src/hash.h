#pragma once

#include "vulkan_types.h"
#include <stdio.h>
#include <unordered_map>

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {

template <>
struct hash<synt::Vec4>
{
    size_t operator()(synt::Vec4 const& v) const
    {
        size_t seed = 0;

        hash_combine(seed, v.x);
        hash_combine(seed, v.y);
        hash_combine(seed, v.z);
        hash_combine(seed, v.w);

        return seed;
    }
};

template <>
struct hash<synt::Vec3>
{
    size_t operator()(synt::Vec3 const& v) const
    {
        size_t seed = 0;

        hash_combine(seed, v.x);
        hash_combine(seed, v.y);
        hash_combine(seed, v.z);

        return seed;
    }
};

template <>
struct hash<synt::Vec2>
{
    size_t operator()(synt::Vec2 const& v) const
    {
        size_t seed = 0;

        hash_combine(seed, v.x);
        hash_combine(seed, v.y);

        return seed;
    }
};

template <>
struct hash<synt::Vertex>
{
    size_t operator()(synt::Vertex const& v) const
    {
        size_t seed = 0;

        hash_combine(seed, v.pos);
        hash_combine(seed, v.color);
        hash_combine(seed, v.tex_coords);
        hash_combine(seed, v.tex_index);

        return seed;
    }
};
} // namespace std
