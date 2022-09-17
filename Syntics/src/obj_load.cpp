#include "obj_load.h"
#include "file_reading.h"
#include "vulkan_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace synt {

Obj_Load_Attrib::Obj_Load_Attrib() : verts(0), normals(0), tex_coords(0), indices(0) {}

void Obj_Load_Attrib::init(size_t size)
{
    bool result = init_region(&m_region, size);
    assert(result);
}

Obj_Load_Attrib::~Obj_Load_Attrib() { free_region(&m_region); }

static void get_floats(const File_Attrib& file, uint32_t& i, float* data, uint32_t size)
{
    if (data == NULL) perror("Data is null");

    char buffer[30]   = {};
    uint32_t buffer_i = 0;

    uint32_t vec_i = 0;

    while (file.buffer.data[i++] != '\n' && i < file.buffer.size())
    {
        if (vec_i == size) continue;
        if (file.buffer.data[i] == ' ') continue;
        buffer_i = 0;
        while (file.buffer.data[i] != ' ' && file.buffer.data[i] != '\n' &&
               i < file.buffer.size())
        {
            buffer[buffer_i++] = file.buffer.data[i++];
        }
        buffer[buffer_i] = '\0';

        data[vec_i++] = (float)atof(buffer);
    }
    i--;
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

static void parse_v(const File_Attrib& file, Vec3* vecs, Vec2* tex_coords, Vec3* normals,
                    uint32_t& i)
{
    switch (file.buffer.data[i])
    {
        case ' ':
        {
            synt_push(vecs, vec3f(file, i));
            break;
        }
        case 't':
        {
            synt_push(tex_coords, vec2f(file, i));
            break;
        }
        case 'n':
        {
            synt_push(normals, vec3f(file, i));
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

static void parse_sizes(const File_Attrib& file, uint32_t& v, uint32_t& vt, uint32_t& vn,
                        uint32_t& f)
{
    v  = 0;
    vt = 0;
    vn = 0;
    f  = 0;
    for (uint32_t i = 0; i < file.buffer.size(); i++)
    {
        if (file.buffer.data[i] == 'v')
        {
            i++;
            switch (file.buffer.data[i])
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
        else if (file.buffer.data[i] == 'f')
        {
            uint32_t points = 1;
            while (file.buffer.data[i++] != '\n' && i < file.buffer.size())
            {
                if (file.buffer.data[i] == ' ') points++;
            }
            f += points - 2;
            continue;
        }
        else
        {
            while (file.buffer.data[i] != '\n')
            {
                i++;
            }
            continue;
        }
    }
}

static void parse_buffer(const File_Attrib& file, Obj_Load_Attrib& obj_attrib)
{
    uint32 v = 0, vt = 0, vn = 0, f = 0;
    parse_sizes(file, v, vt, vn, f);

    obj_attrib

        vecs   = dyn_array((*region), v, Vec3, synt::TEMP_ARRAY);
    tex_coords = dyn_array((*region), vt, Vec2, synt::TEMP_ARRAY);
    normals    = dyn_array((*region), vn, Vec3, synt::TEMP_ARRAY);
    indi       = dyn_array((*region), f, Indices, synt::TEMP_ARRAY);

    for (uint32_t i = 0; i < file.buffer.size(); i++)
    {
        if (file.buffer.data[i] == 'v')
        {
            parse_v(file, vecs, tex_coords, normals, ++i);
            continue;
        }
        else if (file.buffer.data[i] == 'f')
        {
            i++;
            char buffer[30]   = {};
            uint32_t buffer_i = 0;

            uint32_t vec_i        = 0;
            uint32_t indices[100] = {};

            uint32_t points = 1;

            while (file.buffer.data[i++] != '\n' && i < file.buffer.size())
            {
                if (file.buffer.data[i] == '/' || file.buffer.data[i] == ' ') continue;
                buffer_i = 0;
                while (file.buffer.data[i] != '/' && file.buffer.data[i] != ' ' &&
                       file.buffer.data[i] != '\n' && i < file.buffer.size())
                {
                    buffer[buffer_i++] = file.buffer.data[i++];
                }
                buffer[buffer_i] = '\0';

                indices[vec_i++] = (uint32_t)atoi(buffer) - 1;

                if (file.buffer.data[i] == ' ') points++;
            }

            // f: A B C D E F G
            // ABC, ACD, ADE, AEF, AFG.

            for (uint32_t d = 0; d < points - 2; d++)
            {
                uint32_t h   = 0;
                Indices indx = {};
                for (uint32_t j = 0; j < 3; j++)
                {
                    if (j > 0 && d > 0) h = 3 * d;
                    indx.vertex_index[j]  = indices[h + (j * 3)];
                    indx.texture_index[j] = indices[h + 1 + (j * 3)];
                    indx.normals_index[j] = indices[h + 2 + (j * 3)];
                }
                synt_push(indi, indx);
            }

            i--;
            continue;
        }
        else
        {
            while (file.buffer.data[i] != '\n')
            {
                i++;
            }
            continue;
        }
    }
}

} // namespace synt

