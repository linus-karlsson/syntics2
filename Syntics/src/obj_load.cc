#include "obj_load.h"
#include "file_reading.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>

#define GAP(x) (((x) == ' ') || ((x) == '\t'))

internal void _init(u32 v, u32 vn, u32 vt, u32 f, Obj_Load_Attrib* obj_attrib)
{
    b8 result =
        init_region(&obj_attrib->region,
                    (v * sizeof(V3)) + (vn * sizeof(V3)) + (vt * sizeof(V2)) +
                        (f * sizeof(Indices)) + (4 * sizeof(Array_Head)));
    ASSERT(result, "obj_load_init");

    obj_attrib->verts = dyn_array(&obj_attrib->region, v, V3, TEMP_ARRAY);
    obj_attrib->normals = dyn_array(&obj_attrib->region, vn, V3, TEMP_ARRAY);
    obj_attrib->tex_coords = dyn_array(&obj_attrib->region, vt, V2, TEMP_ARRAY);
    obj_attrib->indices = dyn_array(&obj_attrib->region, f, Indices, TEMP_ARRAY);
}

internal void get_floats(const File_Attrib* file, u32* i, float* data, u32 size)
{
    if (data == NULL) perror("Data is null");

    char buffer[30] = { 0 };
    u32 buffer_i = 0;

    u32 vec_i = 0;

    while (file->buffer[(*i)++] != '\n' && (*i) < file->size)
    {
        if (vec_i == size) continue;
        if (GAP(file->buffer[(*i)])) continue;
        buffer_i = 0;
        while (!GAP(file->buffer[(*i)]) && file->buffer[(*i)] != '\n' &&
               (*i) < file->size)
        {
            buffer[buffer_i++] = file->buffer[(*i)++];
        }
        buffer[buffer_i] = '\0';

        data[vec_i++] = (float)atof(buffer);
    }
}

internal V3 vec3f(const File_Attrib* file, u32* i)
{
    V3 vec = v3d();
    f32 vec3f[3] = { 0 };
    get_floats(file, i, vec3f, 3);

    vec.x = vec3f[0];
    vec.y = vec3f[1];
    vec.z = vec3f[2];

    return vec;
}

internal V2 vec2f(const File_Attrib* file, u32* i)
{
    V2 vec = v2d();
    f32 vec2f[2] = { 0 };
    get_floats(file, i, vec2f, 2);

    vec.x = vec2f[0];
    vec.y = vec2f[1];

    return vec;
}

internal void parse_v(const File_Attrib* file, Obj_Load_Attrib* obj_attrib, u32* i)
{
    switch (file->buffer[(*i)])
    {
        case ' ':
        {
            synt_push(obj_attrib->verts, vec3f(file, i));
            break;
        }
        case 't':
        {
            synt_push(obj_attrib->tex_coords, vec2f(file, i));
            break;
        }
        case 'n':
        {
            synt_push(obj_attrib->normals, vec3f(file, i));
            break;
        }
        case 'p':
        {
            break;
        }
        default:
        {
            break;
        }
    }
}

internal void parse_f(const File_Attrib* file, Obj_Load_Attrib* obj_attrib, u32* i)
{
    char buffer[30] = { 0 };
    u32 buffer_i = 0;

    u32 vec_i = 0;
    u32 indi[100] = { 0 };

    u32 points = 1;

    while (file->buffer[(*i)++] != '\n' && (*i) < file->size)
    {
        if (file->buffer[(*i)] == '/' || GAP(file->buffer[(*i)])) continue;
        buffer_i = 0;
        while (file->buffer[(*i)] != '/' && !GAP(file->buffer[(*i)]) &&
               file->buffer[(*i)] != '\n' && (*i) < file->size)
        {
            buffer[buffer_i++] = file->buffer[(*i)++];
        }
        buffer[buffer_i] = '\0';

        indi[vec_i++] = (u32)atoi(buffer) - 1;

        if (GAP(file->buffer[(*i)])) points++;
    }

    for (u32 d = 0; d < points - 2; d++)
    {
        u32 h = 0;
        Indices indx = { 0 };
        for (u32 j = 0; j < 3; j++)
        {
            if (j > 0 && d > 0) h = 3 * d;
            indx.vertex_index[j] = indi[h + (j * 3)];
            indx.texture_index[j] = indi[h + 1 + (j * 3)];
            indx.normals_index[j] = indi[h + 2 + (j * 3)];
        }
        synt_push(obj_attrib->indices, indx);
    }
}

internal void parse_sizes(const File_Attrib* file, u32* v, u32* vt, u32* vn, u32* f)
{
    *v = 0;
    *vt = 0;
    *vn = 0;
    *f = 0;
    for (u32 i = 0; i < file->size; i++)
    {
        if (file->buffer[i] == 'v')
        {
            i++;
            switch (file->buffer[i])
            {
                case ' ':
                {
                    (*v)++;
                    break;
                }
                case 't':
                {
                    (*vt)++;
                    break;
                }
                case 'n':
                {
                    (*vn)++;
                    break;
                }
                case 'p':
                {
                    break;
                }
                default:
                {
                    break;
                }
            }
            continue;
        }
        else if (file->buffer[i] == 'f')
        {
            u32 points = 1;
            while (file->buffer[i++] != '\n' && i < file->size)
            {
                if (GAP(file->buffer[i])) points++;
            }
            (*f) += points - 2;
            continue;
        }
        else
        {
            while (file->buffer[i] != '\n')
            {
                i++;
            }
            continue;
        }
    }
}

internal void _parse_buffer(Obj_Load_Attrib* obj_attrib, const File_Attrib* file)
{
    u32 v = 0, vt = 0, vn = 0, f = 0;
    parse_sizes(file, &v, &vt, &vn, &f);

    _init(v, vn, vt, f, obj_attrib);

    for (u32 i = 0; i < file->size; i++)
    {
        if (file->buffer[i] == 'v')
        {
            ++i;
            parse_v(file, obj_attrib, &i);
            i--;
            continue;
        }
        else if (file->buffer[i] == 'f')
        {
            ++i;
            parse_f(file, obj_attrib, &i);
            i--;
            continue;
        }
        else
        {
            while (file->buffer[i] != '\n')
            {
                i++;
            }
            continue;
        }
    }
}

void load_model(Obj_Load_Attrib* obj_attrib, const char* model_path)
{
    stack_begin_scope();
    File_Attrib file;
    read_file(&file, get_stack(), model_path, "r");

    _parse_buffer(obj_attrib, &file);
    stack_end_scope();
}

void free_obj_load(Obj_Load_Attrib* obj_load)
{
    free_region(&obj_load->region);
}
