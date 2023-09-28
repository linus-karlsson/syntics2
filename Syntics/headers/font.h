#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

typedef struct Character
{
    i32 id;
    i32 x, y;
    i32 width, height;
    i32 x_offset, y_offset;
    i32 x_advance;
} Character;

typedef struct Character_TTF
{
    V2 dimensions;
    V2 offset;
    V4 text_coords;
    f32 x_advance;
} Character_TTF;

typedef struct Font_TTF
{
    f32 tex_index;
    f32 line_height;
    f32 pixel_height;
    u32 char_count;
    Character_TTF* chars;
} Font_TTF;

typedef struct Font
{
    u32 tex_index;
    u32 width_atlas;
    u32 height_atlas;
    u32 line_height;
    u32 pixels;
    u32 num_chars;
    Character* characters;
} Font;

Font font_file_load(Region_Alloc* region, const char* file_path);
V2 altas_coords_to_texidx(f32 x, f32 y, f32 atlas_width, f32 atlas_height);
u32 text_3D(Font font, const char* text, V3 pos_first_letter, f32 size,
            f32 win_width, f32 win_height, Vertex** vertices);
f32 text_x_advance(Font font, const char* text, u32 text_len, f32 size);
u32 text_2D_ttf(Font font, const char* text, V3 pos_first_letter, f32 size,
                Vertex** vertices);
f32 text_x_advance(Font font, const char* text, u32 text_len, f32 size);
u32 text_2D(Font font, f32 y_origin, const char* text, u32 text_len,
            V3 pos_first_letter, V4 color, f32 size, u32* new_lines,
            float* x_adv, Vertex_Array* vert_array);

void init_ttf_atlas(Region_Alloc* region, Font_TTF* font_out, u8* bitmap,
                    i32 width_atlas, i32 height_atlas, f32 pixel_height,
                    u32 glyph_count, u32 glyph_offset,
                    const char* font_file_path);
u32 text_gen(const Character_TTF* c_ttf, const char* text, V3 pos, f32 scale,
             f32 line_height, u32* new_lines_count, f32* x_advance,
             Vertex_Array* array);
