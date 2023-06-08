#pragma once

#include "math/vectors.h"
#include "region_alloc.h"

// TODO: Got some glitches, not many but some.

typedef struct File_Attrib File_Attrib;

typedef struct Indices
{
    u32 vertex_index;
    u32 texture_index;
    u32 normals_index;
} Indices;

typedef struct Obj_Load_Attrib
{

    V3* verts;
    V3* normals;
    V2* tex_coords;

    Indices* indices;

    Region_Alloc region;

} Obj_Load_Attrib;

void load_model(Obj_Load_Attrib* obj_attrib, const char* model_path);
void free_obj_load(Obj_Load_Attrib* obj_load);
