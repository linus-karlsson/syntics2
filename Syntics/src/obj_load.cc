#include "obj_load.h"
#include "file_reading.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GAP(x) (((x) == ' ') || ((x) == '\t'))

internal void _init(u32 v, u32 vn, u32 vt, u32 f, Obj_Load_Attrib* obj_attrib)
{
    b8 result =
        init_region(&obj_attrib->region,
                    (v * sizeof(V3)) + (vn * sizeof(V3)) + (vt * sizeof(V2)) +
                        (f * sizeof(Indices) * 2) + (4 * sizeof(Array_Head)));
    ASSERT(result, "obj_load_init");

    obj_attrib->verts = dyn_array(&obj_attrib->region, v, V3, TEMP_ARRAY);
    obj_attrib->normals = dyn_array(&obj_attrib->region, vn, V3, TEMP_ARRAY);
    obj_attrib->tex_coords = dyn_array(&obj_attrib->region, vt, V2, TEMP_ARRAY);
    obj_attrib->indices = dyn_array(&obj_attrib->region, f, Indices, TEMP_ARRAY);
}

internal void parse_sizes(File_Attrib* file, u32* v, u32* vt, u32* vn, u32* f)
{
    *v = 0;
    *vt = 0;
    *vn = 0;
    *f = 0;

    const char* delims = "\n\r ";
    const u32 max_line_size = 4096;
    char line[max_line_size] = { 0 };
    while (!end_of_file(*file))
    {
        const u32 len = read_line(file, line, max_line_size);
        assert(len < max_line_size);
        if (len < 1) continue;
        char* token = read_token(line, delims);

        if (token[0] == 'v')
        {
            if (!strcmp(token, "v"))
            {
                (*v)++;
            }
            else if (!strcmp(token, "vn"))
            {
                (*vn)++;
            }
            else if (!strcmp(token, "vt"))
            {
                (*vt)++;
            }
        }
        else if (!strcmp(token, "f"))
        {
            for (u32 offset = 2; offset < len; offset++)
            {
                assert(offset < max_line_size);
                if (GAP(line[offset]))
                {
                    (*f)++;
                    while (GAP(line[offset]))
                    {
                        offset++;
                        assert(offset < max_line_size);
                    }
                }
            }
            (*f)++;
        }
    }
}

internal V3 vec3f(const char* line)
{
    V3 vec = v3d();
    str_to_val(line, "%f %f %f", &vec.x, &vec.y, &vec.z);
    return vec;
}

internal V2 vec2f(const char* line)
{
    V2 vec = v2d();
    str_to_val(line, "%f %f", &vec.x, &vec.y);
    return vec;
}

internal void parse_f(Obj_Load_Attrib* obj_attrib, char* line)
{
    u32 i0 = 0, i1 = 0, i2 = 0;

    char* current_pos = line;

    while (*current_pos != '\n' && *current_pos != '\0')
    {
        while (GAP(*current_pos))
        {
            current_pos++;
        }
        if (str_to_val(current_pos, "%u/%u/%u", &i0, &i1, &i2) == 3)
        {
            Indices indices = {};
            indices.vertex_index = i0 - 1;
            indices.texture_index = i1 - 1;
            indices.normals_index = i2 - 1;
            synt_push(obj_attrib->indices, indices);
        }
        else if (str_to_val(current_pos, "%u//%u", &i0, &i1) == 2)
        {
            Indices indices = {};
            indices.vertex_index = i0 - 1;
            indices.normals_index = i1 - 1;
            synt_push(obj_attrib->indices, indices);
        }
        else if (str_to_val(current_pos, "%u/%u", &i0, &i1) == 2)
        {
            Indices indices = {};
            indices.vertex_index = i0 - 1;
            indices.texture_index = i1 - 1;
            synt_push(obj_attrib->indices, indices);
        }
        else if (str_to_val(current_pos, "%u", &i0) == 1)
        {
            Indices indices = {};
            indices.vertex_index = i0 - 1;
            synt_push(obj_attrib->indices, indices);
        }
        while (!GAP(*current_pos) && *current_pos != '\n' && *current_pos != '\0')
        {
            current_pos++;
        }
    }
}

internal void _parse_buffer(Obj_Load_Attrib* obj_attrib, File_Attrib* file)
{
    u32 v = 0, vt = 0, vn = 0, f = 0;
    parse_sizes(file, &v, &vt, &vn, &f);

    _init(v, vn, vt, f, obj_attrib);

    file->current_pos = 0;

    const char* delims = "\n\r ";
    const u32 max_line_size = 4096;
    char line[max_line_size] = { 0 };

    while (!end_of_file(*file))
    {
        const u32 len = read_line(file, line, max_line_size);
        assert(len < max_line_size);
        if (len < 1) continue;
        const char* token = read_token(line, delims);
        if (token[0] == 'v')
        {
            if (!strcmp(token, "v"))
            {
                synt_push(obj_attrib->verts, vec3f(line + 2));
            }
            else if (!strcmp(token, "vn"))
            {
                synt_push(obj_attrib->normals, vec3f(line + 3));
            }
            else if (!strcmp(token, "vt"))
            {
                synt_push(obj_attrib->tex_coords, vec2f(line + 3));
            }
        }
        else if (!strcmp(token, "f"))
        {
            parse_f(obj_attrib, line + 2);
        }
    }
}

void load_model(Obj_Load_Attrib* obj_attrib, const char* model_path)
{
    stack_begin_scope();
    File_Attrib file = {};
    read_file(&file, get_stack(), model_path, "r");

    _parse_buffer(obj_attrib, &file);
    stack_end_scope();
}

void free_obj_load(Obj_Load_Attrib* obj_load)
{
    free_region(&obj_load->region);
}
