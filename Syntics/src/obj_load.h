#pragma once

#include "math/vectors.h"
#include "file_reading.h"

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

    void load_model(const char* model_path);

    Vec3* verts;
    Vec3* normals;
    Vec2* tex_coords;

    Indices* indices;

private:
    void _parse_buffer(const File_Attrib&);
    void _init(uint32 v, uint32 vn, uint32 vt, uint32 f);

    Region_Alloc m_region;

} Obj_Load_Attrib;

} // namespace synt

