#pragma once

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

