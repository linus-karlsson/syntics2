#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#include "collision.h"
#endif

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

typedef struct Selection_Character
{
    char character;
    AABB_2D aabb;
} Selection_Character;

typedef struct Selection_Character_Array
{
    u32 size;
    u32 capacity;
    Selection_Character* data;
} Selection_Character_Array;

typedef struct Colored_Character
{
    V4 color;
    char character;
} Colored_Character;

typedef struct Colored_Character_Array 
{
    u32 size;
    u32 capacity;
    Colored_Character* data;
} Colored_Character_Array;

void init_ttf_atlas(i32 width_atlas, i32 height_atlas, f32 pixel_height, u32 glyph_count, u32 glyph_offset, const char* font_file_path, u8* bitmap, Font_TTF* font_out);

#define text_generation(c_ttf, text, texture_index, pos, scale, line_height, new_lines_count, x_advance, aabbs, array) text_generation_color(c_ttf, text, texture_index, pos, scale, line_height, global_get_text_color(), new_lines_count, x_advance, aabbs, array)
u32 text_generation_color(const Character_TTF* c_ttf, const char* text, float texture_index, V2 pos, f32 scale, f32 line_height, V4 color, u32* new_lines_count, f32* x_advance, Selection_Character_Array* selection_chars, Vertex_2D_Array* array);
u32 text_generation_colored_char(const Character_TTF* c_ttf, const Colored_Character_Array* text, float texture_index, V2 pos, f32 scale, f32 line_height, u32* new_lines_count, f32* x_advance, Selection_Character_Array* selection_chars, Vertex_2D_Array* array);
f32 text_x_advance(const Character_TTF* c_ttf, const char* text, u32 text_len, f32 scale);
i32 text_check_length_within_boundary(const Character_TTF* c_ttf, const char* text, u32 text_len, f32 scale, float boundary);
