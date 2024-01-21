#ifndef SY_UNIT_BUILD
#include "obj_load.h"
#include "math/syntics_math.h"
#include "logging.h"
#include "file_reading.h"
#include "platform.h"
#include <stdio.h>
#endif

#define GAP(x) (((x) == ' ') || ((x) == '\t'))

internal void _init(u32 v, u32 vn, u32 vt, u32 f, Obj_Load_Attrib* obj_attrib)
{
    const u32 padding = 4 * 8;
    b8 result = region_init(&obj_attrib->region,
                            (v * sizeof(V3)) + (vn * sizeof(V3)) +
                                (vt * sizeof(V2)) + (f * sizeof(Indices)) +
                                (4 * sizeof(Array_Head)) + padding);
    assert(result && "obj_load_init");

    obj_attrib->verts = region_array_calloc(&obj_attrib->region, v, V3);
    obj_attrib->normals = region_array_calloc(&obj_attrib->region, vn, V3);
    obj_attrib->tex_coords = region_array_calloc(&obj_attrib->region, vt, V2);
    obj_attrib->indices = region_array_calloc(&obj_attrib->region, f, Indices);
}
#define MAX_LINE_SIZE KILOBYTE(4)

internal void parse_sizes(File_Attrib* file, u32* v, u32* vt, u32* vn, u32* f)
{
    *v = 0;
    *vt = 0;
    *vn = 0;
    *f = 0;

    const char* delims = "\n\r ";
    const u32 max_line_size = MAX_LINE_SIZE;
    char line[MAX_LINE_SIZE] = { 0 };
    while (!end_of_file(file))
    {
        const u32 len = line_read(file, line, max_line_size, false);
        assert(len < max_line_size);
        if (len < 1) continue;
        Token token = token_read(line, len, delims, 3);

        if (token.start)
        {
            if (token.start[0] == 'v')
            {
                if (!strcmp(token.start, "v"))
                {
                    (*v)++;
                }
                else if (!strcmp(token.start, "vn"))
                {
                    (*vn)++;
                }
                else if (!strcmp(token.start, "vt"))
                {
                    (*vt)++;
                }
            }
            else if (!strcmp(token.start, "f"))
            {
                u32 current_count = 0;
                for (u32 offset = 2; offset < len; offset++)
                {
                    assert(offset < max_line_size);
                    if (GAP(line[offset]))
                    {
                        current_count++;
                        while (GAP(line[offset]))
                        {
                            offset++;
                            assert(offset < max_line_size);
                        }
                    }
                }
                ++current_count;
                (*f) += (current_count - 2) * 3;
            }
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

internal void _f_parse(Obj_Load_Attrib* obj_attrib, char* line)
{
    stack_begin_scope(f_parse_stack);
    u32 i0 = 0, i1 = 0, i2 = 0;

    char* current_pos = line;

    Indices* indices_array = stack_array(100, Indices);

    while (*current_pos)
    {
        while (GAP(*current_pos))
        {
            current_pos++;
        }
        Indices indices = { 0 };
        if (str_to_val(current_pos, "%u/%u/%u", &i0, &i1, &i2) == 3)
        {
            indices.vertex_index = i0 - 1;
            indices.texture_index = i1 - 1;
            indices.normal_index = i2 - 1;
            region_array_push(indices_array, indices);
        }
        else if (str_to_val(current_pos, "%u//%u", &i0, &i1) == 2)
        {
            indices.vertex_index = i0 - 1;
            indices.normal_index = i1 - 1;
            region_array_push(indices_array, indices);
        }
        else if (str_to_val(current_pos, "%u/%u", &i0, &i1) == 2)
        {
            indices.vertex_index = i0 - 1;
            indices.texture_index = i1 - 1;
            region_array_push(indices_array, indices);
        }
        else if (str_to_val(current_pos, "%u", &i0) == 1)
        {
            indices.vertex_index = i0 - 1;
            region_array_push(indices_array, indices);
        }
        while (!GAP(*current_pos) && *current_pos != '\0')
        {
            current_pos++;
        }
    }
    const u32 size = array_size(indices_array);

    for (u32 i = 0; i < size - 2; i++)
    {
        u32 h = 0;
        for (u32 j = 0; j < 3; j++)
        {
            if (j > 0 && i > 0) h = i;
            region_array_push(obj_attrib->indices,
                              region_array_value(indices_array, h + j));
        }
    }
    stack_end_scope(f_parse_stack);
}

internal void _buffer_parse(Obj_Load_Attrib* obj_attrib, File_Attrib* file)
{
    u32 v = 0, vt = 0, vn = 0, f = 0;
    parse_sizes(file, &v, &vt, &vn, &f);

    _init(v, vn, vt, f, obj_attrib);

    file->current_pos = 0;

    const char* delims = "\n\r ";
    const u32 max_line_size = MAX_LINE_SIZE;
    char line[MAX_LINE_SIZE] = { 0 };

    while (!end_of_file(file))
    {
        const u32 len = line_read(file, line, max_line_size, true);
        assert(len < max_line_size);
        if (len < 1) continue;
        Token token = token_read(line, len, delims, 3);
        if (token.start)
        {
            if (token.start[0] == 'v')
            {
                if (!strcmp(token.start, "v"))
                {
                    region_array_push(obj_attrib->verts, vec3f(line + 2));
                }
                else if (!strcmp(token.start, "vn"))
                {
                    region_array_push(obj_attrib->normals, vec3f(line + 3));
                }
                else if (!strcmp(token.start, "vt"))
                {
                    region_array_push(obj_attrib->tex_coords, vec2f(line + 3));
                }
            }
            else if (!strcmp(token.start, "f"))
            {
                _f_parse(obj_attrib, line + 2);
            }
        }
    }
}

void model_load(Obj_Load_Attrib* obj_attrib, const char* model_path)
{
    stack_begin_scope(model_load_stack);
    File_Attrib file = { 0 };
    file_read(&file, stack_get(), model_path);

    _buffer_parse(obj_attrib, &file);
    stack_end_scope(model_load_stack);
}

void obj_load_free(Obj_Load_Attrib* obj_load)
{
    region_free(&obj_load->region);
}
