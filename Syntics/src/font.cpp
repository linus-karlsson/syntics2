#include "font.h"
#include "file_reading.h"
#include "logging.h"
#include "region_alloc.h"
#include "vulkan_types.h"
#include "buffers.h"
#include <stb/stb_truetype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RESET(thing, bytes) memset(thing, 0, bytes)
#define MAX_WORD_LEN 30

#define READ_HEADER 0
#define READ_CHARS 1
#define READ_CHAR 2

#define READ_ID 0
#define READ_X 1
#define READ_Y 2
#define READ_WIDTH 3
#define READ_HEIGHT 4
#define READ_X_OFFSET 5
#define READ_Y_OFFSET 6
#define READ_X_ADVANCE 7

namespace synt {

Character::Character()
    : id(0), x(0), y(0), width(0), height(0), x_offset(0), y_offset(0), x_advance(0)
{
}

int32& Character::operator[](int i)
{
    switch (i)
    {
        case READ_ID:
        {
            return id;
        }
        case READ_X:
        {
            return x;
        }
        case READ_Y:
        {
            return y;
        }
        case READ_WIDTH:
        {
            return width;
        }
        case READ_HEIGHT:
        {
            return height;
        }
        case READ_X_OFFSET:
        {
            return x_offset;
        }
        case READ_Y_OFFSET:
        {
            return y_offset;
        }
        case READ_X_ADVANCE:
        {
            return x_advance;
        }
        default:
        {
            ERROR("Index out of bounds!");
        }
    }
    ERROR("Index out of bounds!");
    return id;
}

Font::Font()
    : tex_index(0), width_atlas(0), height_atlas(0), line_height(0), num_chars(0)
{
}

#define get_word(file, index, buffer, new_line)                                     \
    ({                                                                              \
        if (!_get_word(file, index, buffer, new_line))                              \
        {                                                                           \
            end_of_file = true;                                                     \
            break;                                                                  \
        }                                                                           \
    })

static bool _get_word(File_Attrib* file, uint32* index, char* buffer, bool* new_line)
{
    while (file->buffer[*index] == ' ' || file->buffer[*index] == '=')
    {
        if (++(*index) >= file->size) return false;
    }
    uint32_t len = 0;
    while (file->buffer[*index] != ' ' && file->buffer[*index] != '=' &&
           file->buffer[*index] != '\n')
    {
        if (len >= MAX_WORD_LEN) ERROR("WORD TO FLIPPING LONG");
        buffer[len++] = file->buffer[*index];
        if (++(*index) >= file->size) return false;
    }
    buffer[len] = '\0';

    if (file->buffer[*index] == '\n')
        *new_line = true;
    else
        *new_line = false;

    return true;
}

Font load_ftt_file(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, Texture** bitmaps, const char* file_path,
                   float scale)
{

    int w, h, x_off, y_off;
    File_Attrib file = read_file(NULL, "Syntics/res/aakar-medium.ttf", "rb");
    stbtt_fontinfo font;
    stbtt_InitFont(&font, file.buffer, stbtt_GetFontOffsetForIndex(file.buffer, 0));

    uint32 num_textures = size_arr((*bitmaps));

    Font out;

    out.characters = region_mallocP((*region), 128, Character);
    out.pixels     = scale;

    for (uint8 c = 0; c < 128; c++)
    {
        unsigned char* bitmap = stbtt_GetCodepointBitmap(
            &font, 0, stbtt_ScaleForPixelHeight(&font, scale), c, &w, &h, &x_off,
            &y_off);

        w = w < 1 ? 1 : w;
        h = h < 1 ? 1 : h;

        uint32 offset                  = num_textures + c;
        (*bitmaps)[offset].width       = w;
        (*bitmaps)[offset].height      = h;
        (*bitmaps)[offset].size_bytes  = w * h;
        (*bitmaps)[offset].mip_map_lvl = 1;

        create_texture(device, physical_device, command_pool, graphic_queue,
                       VK_FORMAT_R8_SRGB, &(*bitmaps)[offset], bitmap);
        get_head((*bitmaps))->size++;

        free(bitmap);

        Character charac;
        charac.id        = offset;
        charac.width     = w;
        charac.height    = h;
        charac.x_offset  = x_off;
        charac.y_offset  = y_off;
        charac.x_advance = w;

        out.characters[c] = charac;
    }
    return out;
}

Font load_font_file(Region_Alloc* region, const char* file_path)
{
    Font out;
    out.characters     = region_mallocP((*region), 128, Character);
    File_Attrib file   = read_file(region, file_path, "r");
    uint32_t value_len = 0;
    char word[MAX_WORD_LEN];
    RESET(word, sizeof(word));

    uint32_t total_num_chars = 0;
    uint32_t mode            = 0;

    bool header_read = false, new_line = false, end_of_file = false;
    for (uint32_t i = 0; i < file.size; i++)
    {
        while (file.buffer[i] != '\n')
        {
            get_word(&file, &i, word, &new_line);
            if (!header_read)
            {
                if (!strcmp(word, "chars"))
                {
                    get_word(&file, &i, word, &new_line);
                    mode = READ_CHARS;
                }
                else if (!strcmp(word, "char"))
                {
                    header_read = true;
                    mode        = READ_CHAR;
                }
                else if (!strcmp(word, "scaleW"))
                {
                    get_word(&file, &i, word, &new_line);
                    out.width_atlas = atoi(word);
                }
                else if (!strcmp(word, "scaleH"))
                {
                    get_word(&file, &i, word, &new_line);
                    out.height_atlas = atoi(word);
                }
                else if (!strcmp(word, "lineHeight"))
                {
                    get_word(&file, &i, word, &new_line);
                    out.line_height = atoi(word);
                }
            }
            if (i >= file.size - 1)
            {
                end_of_file = true;
                break;
            }
            switch (mode)
            {
                case READ_HEADER:
                {
                    break;
                }
                case READ_CHARS:
                {
                    if (!strcmp(word, "count"))
                    {
                        get_word(&file, &i, word, &new_line);
                        total_num_chars = atoi(word);
                        break;
                    }
                }
                case READ_CHAR:
                {
                    if (strcmp(word, "char"))
                    {
                        end_of_file = true;
                        break;
                    }
                    int counter = 0;
                    int id      = 0;
                    while (!new_line)
                    {
                        get_word(&file, &i, word, &new_line);
                        if (counter <= READ_X_ADVANCE)
                        {
                            get_word(&file, &i, word, &new_line);
                            if (counter == READ_ID)
                            {
                                id = atoi(word);
                                out.num_chars++;
                            }
                            out.characters[id][counter++] = atoi(word);
                        }
                    }
                    if (out.num_chars == total_num_chars)
                    {
                        end_of_file = true;
                        break;
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (end_of_file) break;
            if (new_line) continue;
            i++;
        }
        if (end_of_file) break;
    }
    region_pop((*region), file.size, unsigned char, TEMP_MALLOC);
    return out;
}

Vec2 altas_coords_to_texidx(float x, float y, float atlas_width, float atlas_height)
{
    Vec2 out;
    out.x = x / atlas_width;
    out.y = y / atlas_height;
    return out;
}

uint32 text_3D(Font font, const char* text, Vec3 pos_first_letter, float size,
               float win_width, float win_height, Vertex** vertices)
{
    if (!vertices) ERROR("vertices can't be null");

    // Pos from top left corner (0, 0)
    const float x_start = -1.0f;
    const float y_start = -1.0f;

    float x_advance         = 0.0f;
    float y_advance         = 0.0f;
    const float line_height = (float)font.line_height;
    const size_t text_len   = strlen(text);

    for (size_t i = 0; i < text_len; i++)
    {
        if (text[i] == '\n')
        {
            x_advance = 0;
            y_advance += line_height * size;
            continue;
        }
        const Character curr_char = font.characters[text[i]];
        const float char_height   = (float)curr_char.height;
        const float char_width    = (float)curr_char.width;
        const float x_offset      = (float)curr_char.x_offset * size;
        const float y_offset      = (float)curr_char.y_offset * size;
        const float x             = (float)curr_char.x;
        const float y             = (float)curr_char.y;
        const float atlas_width   = (float)font.width_atlas;
        const float atlas_heigth  = (float)font.height_atlas;

        Vertex verts[4];
        verts[0].pos.x =
            x_start +
            (((pos_first_letter.x * 2) + x_offset + x_advance) / win_width);
        verts[0].pos.y =
            y_start +
            (((pos_first_letter.y * 2) + y_offset + y_advance) / win_height);
        verts[0].pos.z = pos_first_letter.z;
        verts[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        verts[0].tex_coords =
            altas_coords_to_texidx(x, y, atlas_width, atlas_heigth);
        verts[0].tex_index = font.tex_index;

        verts[1].pos.x =
            x_start +
            (((pos_first_letter.x * 2) + x_offset + x_advance) / win_width);
        verts[1].pos.y = y_start + (((pos_first_letter.y * 2) + y_offset +
                                     y_advance + (char_height * size)) /
                                    win_height);
        verts[1].pos.z = pos_first_letter.z;
        verts[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        verts[1].tex_coords =
            altas_coords_to_texidx(x, y + char_height, atlas_width, atlas_heigth);
        verts[1].tex_index = font.tex_index;

        verts[2].pos.x      = x_start + (((pos_first_letter.x * 2) + x_offset +
                                     x_advance + (char_width * size)) /
                                    win_width);
        verts[2].pos.y      = y_start + (((pos_first_letter.y * 2) + y_offset +
                                     y_advance + (char_height * size)) /
                                    win_height);
        verts[2].pos.z      = pos_first_letter.z;
        verts[2].color      = { 1.0f, 1.0f, 1.0f, 1.0f };
        verts[2].tex_coords = altas_coords_to_texidx(x + char_width, y + char_height,
                                                     atlas_width, atlas_heigth);
        verts[2].tex_index  = font.tex_index;

        verts[3].pos.x = x_start + (((pos_first_letter.x * 2) + x_offset +
                                     x_advance + (char_width * size)) /
                                    win_width);
        verts[3].pos.y =
            y_start +
            (((pos_first_letter.y * 2) + y_offset + y_advance) / win_height);
        verts[3].pos.z = pos_first_letter.z;
        verts[3].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        verts[3].tex_coords =
            altas_coords_to_texidx(x + char_width, y, atlas_width, atlas_heigth);
        verts[3].tex_index = font.tex_index;

        for (uint32 i = 0; i < 4; i++)
            synt_push((*vertices), verts[i]);

        x_advance += (float)curr_char.x_advance * size;
    }
    return text_len;
}

uint32 text_2D_ttf(Font font, const char* text, Vec3 pos_first_letter, float size,
                   Vertex** vertices)
{
    if (!vertices) ERROR("vertices can't be null");

    float x_advance         = 0.0f;
    float y_advance         = 0.0f;
    const float line_height = (float)font.line_height;
    const size_t text_len   = strlen(text);

    const float pos_y = pos_first_letter.y + (font.pixels * size);

    for (size_t i = 0; i < text_len; i++)
    {
        if (text[i] == '\n')
        {
            x_advance = 0;
            y_advance += line_height * size;
            continue;
        }
        const Character* curr_char = &font.characters[text[i]];
        const float char_height    = (float)curr_char->height;
        const float char_width     = (float)curr_char->width;
        const float x_offset       = (float)(curr_char->x_offset) * size;
        const float y_offset       = (float)(curr_char->y_offset) * size;

        Vertex verts[4];
        verts[0].pos.x      = pos_first_letter.x + x_offset + x_advance;
        verts[0].pos.y      = pos_y + y_offset + y_advance;
        verts[0].pos.z      = pos_first_letter.z;
        verts[0].pos.w      = 1.0f;
        verts[0].color      = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
        verts[0].tex_coords = Vec2(0.0f);
        verts[0].tex_index  = curr_char->id;

        verts[1].pos.x      = pos_first_letter.x + x_offset + x_advance;
        verts[1].pos.y      = pos_y + y_offset + y_advance + (char_height * size);
        verts[1].pos.z      = pos_first_letter.z;
        verts[1].pos.w      = 1.0f;
        verts[1].color      = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
        verts[1].tex_coords = Vec2(0.0f, 1.0f);
        verts[1].tex_index  = curr_char->id;

        verts[2].pos.x =
            pos_first_letter.x + x_offset + x_advance + (char_width * size);
        verts[2].pos.y      = pos_y + y_offset + y_advance + (char_height * size);
        verts[2].pos.z      = pos_first_letter.z;
        verts[2].pos.w      = 1.0f;
        verts[2].color      = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
        verts[2].tex_coords = Vec2(1.0f);
        verts[2].tex_index  = curr_char->id;

        verts[3].pos.x =
            pos_first_letter.x + x_offset + x_advance + (char_width * size);
        verts[3].pos.y      = pos_y + y_offset + y_advance;
        verts[3].pos.z      = pos_first_letter.z;
        verts[3].pos.w      = 1.0f;
        verts[3].color      = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
        verts[3].tex_coords = Vec2(1.0f, 0.0f);
        verts[3].tex_index  = curr_char->id;

        for (uint32 i = 0; i < 4; i++)
        {
            synt_push((*vertices), verts[i]);
        }

        x_advance += (float)curr_char->x_advance * size;
    }
    return text_len;
}

uint32 text_2D(Font font, const char* text, Vec3 pos_first_letter, float size,
               Vertex** vertices)
{
    if (!vertices) ERROR("vertices can't be null");

    float x_advance         = 0.0f;
    float y_advance         = 0.0f;
    const float line_height = (float)font.line_height;
    const size_t text_len   = strlen(text);

    for (size_t i = 0; i < text_len; i++)
    {
        if (text[i] == '\n')
        {
            x_advance = 0;
            y_advance += line_height * size;
            continue;
        }
        const Character curr_char = font.characters[text[i]];
        const float char_height   = (float)curr_char.height;
        const float char_width    = (float)curr_char.width;
        const float x_offset      = (float)curr_char.x_offset * size;
        const float y_offset      = (float)curr_char.y_offset * size;
        const float x             = (float)curr_char.x;
        const float y             = (float)curr_char.y;
        const float atlas_width   = (float)font.width_atlas;
        const float atlas_heigth  = (float)font.height_atlas;

        Vertex verts[4];
        verts[0].pos.x = pos_first_letter.x + x_offset + x_advance;
        verts[0].pos.y = pos_first_letter.y + y_offset + y_advance;
        verts[0].pos.z = pos_first_letter.z;
        verts[0].pos.w = 1.0f;
        verts[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        verts[0].tex_coords =
            altas_coords_to_texidx(x, y, atlas_width, atlas_heigth);
        verts[0].tex_index = font.tex_index;

        verts[1].pos.x = pos_first_letter.x + x_offset + x_advance;
        verts[1].pos.y =
            pos_first_letter.y + y_offset + y_advance + (char_height * size);
        verts[1].pos.z = pos_first_letter.z;
        verts[1].pos.w = 1.0f;
        verts[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        verts[1].tex_coords =
            altas_coords_to_texidx(x, y + char_height, atlas_width, atlas_heigth);
        verts[1].tex_index = font.tex_index;

        verts[2].pos.x =
            pos_first_letter.x + x_offset + x_advance + (char_width * size);
        verts[2].pos.y =
            pos_first_letter.y + y_offset + y_advance + (char_height * size);
        verts[2].pos.z      = pos_first_letter.z;
        verts[2].pos.w      = 1.0f;
        verts[2].color      = { 1.0f, 1.0f, 1.0f, 1.0f };
        verts[2].tex_coords = altas_coords_to_texidx(x + char_width, y + char_height,
                                                     atlas_width, atlas_heigth);
        verts[2].tex_index  = font.tex_index;

        verts[3].pos.x =
            pos_first_letter.x + x_offset + x_advance + (char_width * size);
        verts[3].pos.y = pos_first_letter.y + y_offset + y_advance;
        verts[3].pos.z = pos_first_letter.z;
        verts[3].pos.w = 1.0f;
        verts[3].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        verts[3].tex_coords =
            altas_coords_to_texidx(x + char_width, y, atlas_width, atlas_heigth);
        verts[3].tex_index = font.tex_index;

        for (uint32 i = 0; i < 4; i++)
            synt_push((*vertices), verts[i]);

        x_advance += (float)curr_char.x_advance * size;
    }
    return text_len;
}

} // namespace synt
