#pragma once

#include "defines.h"
#include "vulkan_types.h"
#include "region_alloc.h"

namespace synt {

typedef struct Indices
{
    uint32 vertex_index[3];
    uint32 texture_index[3];
    uint32 normals_index[3];
} Indices;

typedef struct Obj_Load_Attrib
{
    Obj_Load_Attrib();
    ~Obj_Load_Attrib();

    void init(size_t size);

    Vec3* verts;
    Vec3* normals;
    Vec2* tex_coords;

    Indices* indices;

private:
    Region_Alloc m_region;

} Obj_Load_Attrib;

} // namespace synt

