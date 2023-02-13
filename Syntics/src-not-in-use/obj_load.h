#pragma once

#include "math/vectors.h"
#include "region_alloc.h"

namespace synt {

// TODO: Got some glitches, not many but some.

typedef struct File_Attrib File_Attrib;

typedef struct Indices
{
    u32vertex_index[3];
    u32texture_index[3];
    u32normals_index[3];
} Indices;

typedef struct Obj_Load_Attrib
{
    Obj_Load_Attrib();
    ~Obj_Load_Attrib();

    void load_model(const char* model_path);

    Vec3* verts;
    Vec3* normals;
    Vec2* tex_coords;

    Indices* indices;

private:
    void _parse_buffer(const File_Attrib&);
    void _init(u32v, u32vn, u32vt, u32f);

    Region_Alloc m_region;

} Obj_Load_Attrib;

} // namespace synt

