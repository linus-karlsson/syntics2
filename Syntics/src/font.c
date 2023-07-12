
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

i32* set_char(Character* chars, i32 i)
{
    switch (i)
    {
        case READ_ID:
        {
            return &chars->id;
        }
        case READ_X:
        {
            return &chars->x;
        }
        case READ_Y:
        {
            return &chars->y;
        }
        case READ_WIDTH:
        {
            return &chars->width;
        }
        case READ_HEIGHT:
        {
            return &chars->height;
        }
        case READ_X_OFFSET:
        {
            return &chars->x_offset;
        }
        case READ_Y_OFFSET:
        {
            return &chars->y_offset;
        }
        case READ_X_ADVANCE:
        {
            return &chars->x_advance;
        }
        default:
        {
            SY_ERROR("Index out of bounds!");
        }
    }
    return &chars->id;
}

#define get_word(file, index, buffer, new_line)                                     \
    do                                                                              \
    {                                                                               \
        if (!_get_word(file, index, buffer, new_line))                              \
        {                                                                           \
            end_of_file = true;                                                     \
            break;                                                                  \
        }                                                                           \
    } while (0)

static b8 _get_word(File_Attrib* file, u32* index, char* buffer, b8* new_line)
{
    while (file->buffer[*index] == ' ' || file->buffer[*index] == '=')
    {
        if (++(*index) >= file->size) return false;
    }
    u32 len = 0;
    while (file->buffer[*index] != ' ' && file->buffer[*index] != '=' &&
           file->buffer[*index] != '\n')
    {
        if (len >= MAX_WORD_LEN) SY_ERROR("WORD TO FLIPPING LONG");
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

#if 0
#include "buffers.h"
#include <stb/stb_truetype.h>

Font load_ftt_file(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, Texture** bitmaps, const char* file_path,
                   f32 scale)
{

    int w, h, x_off, y_off;
    File_Attrib file;
    read_file(file, NULL, "Syntics/res/aakar-medium.ttf", "rb");
    stbtt_fontinfo font;
    stbtt_InitFont(&font, file.buffer, stbtt_GetFontOffsetForIndex(file.buffer, 0));

    u32 num_textures = size_arr((*bitmaps));

    Font out;

    out.characters = region_mallocP(region, 128, Character);
    out.pixels = scale;

    for (u8 c = 0; c < 128; c++)
    {
        unsigned char* bitmap = stbtt_GetCodepointBitmap(
            &font, 0, stbtt_ScaleForPixelHeight(&font, scale), c, &w, &h, &x_off,
            &y_off);

        w = w < 1 ? 1 : w;
        h = h < 1 ? 1 : h;

        u32 offset = num_textures + c;
        (*bitmaps)[offset].width = w;
        (*bitmaps)[offset].height = h;
        (*bitmaps)[offset].size_bytes = w * h;
        (*bitmaps)[offset].mip_map_lvl = 1;

        create_texture(device, physical_device, command_pool, graphic_queue,
                       VK_FORMAT_R8_SRGB, &(*bitmaps)[offset], bitmap);
        get_head((*bitmaps))->size++;

        free(bitmap);

        Character charac;
        charac.id = offset;
        charac.width = w;
        charac.height = h;
        charac.x_offset = x_off;
        charac.y_offset = y_off;
        charac.x_advance = w;

        out.characters[c] = charac;
    }
    return out;
}
#endif

Font load_font_file(Region_Alloc* region, const char* file_path)
{
    stack_begin_scope();
    Font out = {  0};
    ASSERT(out.characters == NULL, "");
    out.characters = region_mallocP(region, 128, Character);
    for(u32 i = 0; i <  128; i++)
    {
        memset(&out.characters[i], 0, sizeof(out.characters[i]));
    }
    const char* full_path = extend_path_d1(file_path);
    File_Attrib file = { 0 };
    read_file(&file, get_stack(), full_path, "r");
    char word[MAX_WORD_LEN] = { 0 };

    u32 total_num_chars = 0;
    u32 mode = 0;

    b8 header_read = false, new_line = false, end_of_file = false;
    for (u32 i = 0; i < file.size; i++)
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
                    mode = READ_CHAR;
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
                    int id = 0;
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
                            *set_char(&out.characters[id], counter++) = atoi(word);
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
    stack_end_scope();
    return out;
}

V2 altas_coords_to_texidx(f32 x, f32 y, f32 atlas_width, f32 atlas_height)
{
    V2 out;
    out.x = x / atlas_width;
    out.y = y / atlas_height;
    return out;
}

u32 text_3D(Font font, const char* text, V3 pos_first_letter, f32 size,
            f32 win_width, f32 win_height, Vertex** vertices)
{
    if (!vertices) SY_ERROR("vertices can't be null");

    // Pos from top left corner (0, 0)
    const f32 x_start = -1.0f;
    const f32 y_start = -1.0f;

    f32 x_advance = 0.0f;
    f32 y_advance = 0.0f;
    const f32 line_height = (float)font.line_height;
    const size_t text_len = strlen(text);

    for (size_t i = 0; i < text_len; i++)
    {
        if (text[i] == '\n')
        {
            x_advance = 0;
            y_advance += line_height * size;
            continue;
        }
        const Character curr_char = font.characters[(size_t)text[i]];
        const f32 char_height = (float)curr_char.height;
        const f32 char_width = (float)curr_char.width;
        const f32 x_offset = (float)curr_char.x_offset * size;
        const f32 y_offset = (float)curr_char.y_offset * size;
        const f32 x = (float)curr_char.x;
        const f32 y = (float)curr_char.y;
        const f32 atlas_width = (float)font.width_atlas;
        const f32 atlas_heigth = (float)font.height_atlas;

        Vertex verts[4];
        verts[0].pos.x =
            x_start +
            (((pos_first_letter.x * 2) + x_offset + x_advance) / win_width);
        verts[0].pos.y =
            y_start +
            (((pos_first_letter.y * 2) + y_offset + y_advance) / win_height);
        verts[0].pos.z = pos_first_letter.z;
        verts[0].color = v4f(1.0f, 1.0f, 1.0f, 1.0f);
        verts[0].tex_coords =
            altas_coords_to_texidx(x, y, atlas_width, atlas_heigth);
        verts[0].tex_index = (f32)font.tex_index;

        verts[1].pos.x =
            x_start +
            (((pos_first_letter.x * 2) + x_offset + x_advance) / win_width);
        verts[1].pos.y = y_start + (((pos_first_letter.y * 2) + y_offset +
                                     y_advance + (char_height * size)) /
                                    win_height);
        verts[1].pos.z = pos_first_letter.z;
        verts[1].color = v4f(1.0f, 1.0f, 1.0f, 1.0f);
        verts[1].tex_coords =
            altas_coords_to_texidx(x, y + char_height, atlas_width, atlas_heigth);
        verts[1].tex_index = (f32)font.tex_index;

        verts[2].pos.x = x_start + (((pos_first_letter.x * 2) + x_offset +
                                     x_advance + (char_width * size)) /
                                    win_width);
        verts[2].pos.y = y_start + (((pos_first_letter.y * 2) + y_offset +
                                     y_advance + (char_height * size)) /
                                    win_height);
        verts[2].pos.z = pos_first_letter.z;
        verts[2].color = v4f(1.0f, 1.0f, 1.0f, 1.0f);
        verts[2].tex_coords = altas_coords_to_texidx(x + char_width, y + char_height,
                                                     atlas_width, atlas_heigth);
        verts[2].tex_index = (f32)font.tex_index;

        verts[3].pos.x = x_start + (((pos_first_letter.x * 2) + x_offset +
                                     x_advance + (char_width * size)) /
                                    win_width);
        verts[3].pos.y =
            y_start +
            (((pos_first_letter.y * 2) + y_offset + y_advance) / win_height);
        verts[3].pos.z = pos_first_letter.z;
        verts[3].color = v4f(1.0f, 1.0f, 1.0f, 1.0f);
        verts[3].tex_coords =
            altas_coords_to_texidx(x + char_width, y, atlas_width, atlas_heigth);
        verts[3].tex_index = (f32)font.tex_index;

        for (u32 j = 0; j < 4; j++)
            array_push((*vertices), verts[j]);

        x_advance += (float)curr_char.x_advance * size;
    }
    return (u32)text_len;
}

f32 text_x_advance(Font font, const char* text, u32 text_len, f32 size)
{
    f32 result = 0;
    for(u32 i = 0; i < text_len; i++)
    {
        result += font.characters[(size_t)text[i]].x_advance * size;
    }
    return result;
}

u32 text_2D_ttf(Font font, const char* text, V3 pos_first_letter, f32 size,
                Vertex** vertices)
{
    if (!vertices) SY_ERROR("vertices can't be null");

    f32 x_advance = 0.0f;
    f32 y_advance = 0.0f;
    const f32 line_height = (float)font.line_height;
    const size_t text_len = strlen(text);

    const f32 pos_y = pos_first_letter.y + (font.pixels * size);

    for (size_t i = 0; i < text_len; i++)
    {
        if (text[i] == '\n')
        {
            x_advance = 0;
            y_advance += line_height * size;
            continue;
        }
        const Character* curr_char = &font.characters[(size_t)text[i]];
        const f32 char_height = (float)curr_char->height;
        const f32 char_width = (float)curr_char->width;
        const f32 x_offset = (float)(curr_char->x_offset) * size;
        const f32 y_offset = (float)(curr_char->y_offset) * size;

        Vertex verts[4];
        verts[0].pos.x = pos_first_letter.x + x_offset + x_advance;
        verts[0].pos.y = pos_y + y_offset + y_advance;
        verts[0].pos.z = pos_first_letter.z;
        verts[0].color = v4i(1.0f);
        verts[0].tex_coords = v2i(0.0f);
        verts[0].tex_index = (f32)curr_char->id;

        verts[1].pos.x = pos_first_letter.x + x_offset + x_advance;
        verts[1].pos.y = pos_y + y_offset + y_advance + (char_height * size);
        verts[1].pos.z = pos_first_letter.z;
        verts[1].color = v4i(1.0f);
        verts[1].tex_coords = v2f(0.0f, 1.0f);
        verts[1].tex_index = (f32)curr_char->id;

        verts[2].pos.x =
            pos_first_letter.x + x_offset + x_advance + (char_width * size);
        verts[2].pos.y = pos_y + y_offset + y_advance + (char_height * size);
        verts[2].pos.z = pos_first_letter.z;
        verts[2].color = v4i(1.0f);
        verts[2].tex_coords = v2i(1.0f);
        verts[2].tex_index = (f32)curr_char->id;

        verts[3].pos.x =
            pos_first_letter.x + x_offset + x_advance + (char_width * size);
        verts[3].pos.y = pos_y + y_offset + y_advance;
        verts[3].pos.z = pos_first_letter.z;
        verts[3].color = v4i(1.0f);
        verts[3].tex_coords = v2f(1.0f, 0.0f);
        verts[3].tex_index = (f32)curr_char->id;

        for (u32 j = 0; j < 4; j++)
        {
            array_push((*vertices), verts[j]);
        }

        x_advance += (float)curr_char->x_advance * size;
    }
    return (u32)text_len;
}

u32 text_2D(Font font, f32 y_origin, const char* text, u32 text_len,
            V3 pos_first_letter, V4 color, f32 size, u32* new_lines,
            float* x_adv, Vertex* vertices)
{
    if (!vertices) SY_ERROR("vertices can't be null");

    f32 y_o = 1.0f;
    if (y_origin < 0.0f)
    {
        y_o = -1.0f;
    }

    u32 result = 0;

    f32 x_advance = 0.0f;
    f32 y_advance = 0.0f;
    const f32 line_height = (float)font.line_height;

    f32 total_x_advance = 0;
    u32 new_lines_count = 0;

    for (u32 i = 0; i < text_len; i++)
    {
        if (text[i] == '\n')
        {
            total_x_advance =
                x_advance > total_x_advance ? x_advance : total_x_advance;
            x_advance = 0;
            y_advance += line_height * size;
            new_lines_count++;
            continue;
        }
        const Character curr_char = font.characters[(size_t)text[i]];
        const f32 char_height = (float)curr_char.height;
        const f32 char_width = (float)curr_char.width;
        const f32 x_offset = (float)curr_char.x_offset * size;
        const f32 y_offset = (float)curr_char.y_offset * size;
        const f32 x = (float)curr_char.x;
        const f32 y = (float)curr_char.y;
        const f32 atlas_width = (float)font.width_atlas;
        const f32 atlas_heigth = (float)font.height_atlas;

        Vertex verts[4];
        verts[0].pos.x = pos_first_letter.x + x_offset + x_advance;
        verts[0].pos.y = pos_first_letter.y + ((y_offset + y_advance) * y_o);
        verts[0].pos.z = pos_first_letter.z;
        verts[0].color = color;
        verts[0].tex_coords =
            altas_coords_to_texidx(x, y, atlas_width, atlas_heigth);
        verts[0].tex_index = (f32)font.tex_index;

        verts[1].pos.x = pos_first_letter.x + x_offset + x_advance;
        verts[1].pos.y = pos_first_letter.y +
                         ((y_offset + y_advance + (char_height * size)) * y_o);
        verts[1].pos.z = pos_first_letter.z;
        verts[1].color = color;
        verts[1].tex_coords =
            altas_coords_to_texidx(x, y + char_height, atlas_width, atlas_heigth);
        verts[1].tex_index = (f32)font.tex_index;

        verts[2].pos.x =
            pos_first_letter.x + x_offset + x_advance + (char_width * size);
        verts[2].pos.y = pos_first_letter.y +
                         ((y_offset + y_advance + (char_height * size)) * y_o);
        verts[2].pos.z = pos_first_letter.z;
        verts[2].color = color;
        verts[2].tex_coords = altas_coords_to_texidx(x + char_width, y + char_height,
                                                     atlas_width, atlas_heigth);
        verts[2].tex_index = (f32)font.tex_index;

        verts[3].pos.x =
            pos_first_letter.x + x_offset + x_advance + (char_width * size);
        verts[3].pos.y = pos_first_letter.y + ((y_offset + y_advance) * y_o);
        verts[3].pos.z = pos_first_letter.z;
        verts[3].color = color;
        verts[3].tex_coords =
            altas_coords_to_texidx(x + char_width, y, atlas_width, atlas_heigth);
        verts[3].tex_index = (f32)font.tex_index;

        for (u32 j = 0; j < 4; j++)
            array_push(vertices, verts[j]);

        x_advance += (float)curr_char.x_advance * size;
        result++;
    }
    if (new_lines)
    {
        *new_lines += new_lines_count;
    }
    if (x_adv)
    {
        *x_adv = x_advance > total_x_advance ? x_advance : total_x_advance;
    }
    return result;
}
