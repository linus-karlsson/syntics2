#include "obj_load.h"
#include "file_reading.h"
#include "vulkan_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace synt {

#define GAP(x) (((x) == ' ') || ((x) == '\t'))

Obj_Load_Attrib::Obj_Load_Attrib()
    : verts(0), normals(0), tex_coords(0), indices(0)
{
}

void Obj_Load_Attrib::_init(uint32 v, uint32 vn, uint32 vt, uint32 f)
{
    bool result =
        init_region(&m_region, (v * sizeof(Vec3)) + (vn * sizeof(Vec3)) +
                                   (vt * sizeof(Vec2)) + (f * sizeof(Indices)) +
                                   (4 * sizeof(Array_Head)));
    assert(result);

    verts      = dyn_array(m_region, v, Vec3, TEMP_ARRAY);
    normals    = dyn_array(m_region, vn, Vec3, TEMP_ARRAY);
    tex_coords = dyn_array(m_region, vt, Vec2, TEMP_ARRAY);
    indices    = dyn_array(m_region, f, Indices, TEMP_ARRAY);
}

Obj_Load_Attrib::~Obj_Load_Attrib() { free_region(&m_region); }

static void get_floats(const File_Attrib& file, uint32_t& i, float* data,
                       uint32_t size)
{
    if (data == NULL) perror("Data is null");

    char buffer[30]   = {};
    uint32_t buffer_i = 0;

    uint32_t vec_i = 0;

    while (file.buffer[i++] != '\n' && i < file.size)
    {
        if (vec_i == size) continue;
        if (GAP(file.buffer[i])) continue;
        buffer_i = 0;
        while (!GAP(file.buffer[i]) && file.buffer[i] != '\n' && i < file.size)
        {
            buffer[buffer_i++] = file.buffer[i++];
        }
        buffer[buffer_i] = '\0';

        data[vec_i++] = (float)atof(buffer);
    }
}

static Vec3 vec3f(const File_Attrib& file, uint32_t& i)
{
    Vec3 vec       = {};
    float vec3f[3] = {};
    get_floats(file, i, vec3f, 3);

    vec.x = vec3f[0];
    vec.y = vec3f[1];
    vec.z = vec3f[2];

    return vec;
}

static Vec2 vec2f(const File_Attrib& file, uint32_t& i)
{
    Vec2 vec       = {};
    float vec2f[2] = {};
    get_floats(file, i, vec2f, 2);

    vec.x = vec2f[0];
    vec.y = vec2f[1];

    return vec;
}

static void parse_v(const File_Attrib& file, Obj_Load_Attrib* obj_attrib,
                    uint32_t& i)
{
    switch (file.buffer[i])
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

static void parse_f(const File_Attrib& file, Obj_Load_Attrib* obj_attrib,
                    uint32_t& i)
{
    char buffer[30]   = {};
    uint32_t buffer_i = 0;

    uint32_t vec_i     = 0;
    uint32_t indi[100] = {};

    uint32_t points = 1;

    while (file.buffer[i++] != '\n' && i < file.size)
    {
        if (file.buffer[i] == '/' || GAP(file.buffer[i])) continue;
        buffer_i = 0;
        while (file.buffer[i] != '/' && !GAP(file.buffer[i]) &&
               file.buffer[i] != '\n' && i < file.size)
        {
            buffer[buffer_i++] = file.buffer[i++];
        }
        buffer[buffer_i] = '\0';

        indi[vec_i++] = (uint32_t)atoi(buffer) - 1;

        if (GAP(file.buffer[i])) points++;
    }

    for (uint32_t d = 0; d < points - 2; d++)
    {
        uint32_t h   = 0;
        Indices indx = {};
        for (uint32_t j = 0; j < 3; j++)
        {
            if (j > 0 && d > 0) h = 3 * d;
            indx.vertex_index[j]  = indi[h + (j * 3)];
            indx.texture_index[j] = indi[h + 1 + (j * 3)];
            indx.normals_index[j] = indi[h + 2 + (j * 3)];
        }
        synt_push(obj_attrib->indices, indx);
    }
}

static void parse_sizes(const File_Attrib& file, uint32_t& v, uint32_t& vt,
                        uint32_t& vn, uint32_t& f)
{
    v  = 0;
    vt = 0;
    vn = 0;
    f  = 0;
    for (uint32_t i = 0; i < file.size; i++)
    {
        if (file.buffer[i] == 'v')
        {
            i++;
            switch (file.buffer[i])
            {
                case ' ':
                {
                    v++;
                    break;
                }
                case 't':
                {
                    vt++;
                    break;
                }
                case 'n':
                {
                    vn++;
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
        else if (file.buffer[i] == 'f')
        {
            uint32_t points = 1;
            while (file.buffer[i++] != '\n' && i < file.size)
            {
                if (GAP(file.buffer[i])) points++;
            }
            f += points - 2;
            continue;
        }
        else
        {
            while (file.buffer[i] != '\n')
            {
                i++;
            }
            continue;
        }
    }
}

void Obj_Load_Attrib::_parse_buffer(const File_Attrib& file)
{
    uint32 v = 0, vt = 0, vn = 0, f = 0;
    parse_sizes(file, v, vt, vn, f);

    _init(v, vn, vt, f);

    for (uint32_t i = 0; i < file.size; i++)
    {
        if (file.buffer[i] == 'v')
        {
            parse_v(file, this, ++i);
            i--;
            continue;
        }
        else if (file.buffer[i] == 'f')
        {
            parse_f(file, this, ++i);
            i--;
            continue;
        }
        else
        {
            while (file.buffer[i] != '\n')
            {
                i++;
            }
            continue;
        }
    }
}

void Obj_Load_Attrib::load_model(const char* model_path)
{
    File_Attrib file = read_file(NULL, model_path, "r");

    _parse_buffer(file);
}

} // namespace synt

