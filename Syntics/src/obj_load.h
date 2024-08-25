#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#include "region_alloc.h"
#endif

typedef struct Indices
{
    u32 vertex_index; 
    u32 texture_index; 
    u32 normal_index; 
} Indices;

typedef struct Obj_Load_Attrib
{
    V3* verts;
    V3* normals;
    V2* tex_coords;

    Indices* indices;

    Region_Alloc region;
} Obj_Load_Attrib;

void model_load(Obj_Load_Attrib* obj_attrib, const char* model_path);
void obj_load_free(Obj_Load_Attrib* obj_load);
