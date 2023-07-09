#pragma once

typedef struct Character
{
    i32 id;
    i32 x, y;
    i32 width, height;
    i32 x_offset, y_offset;
    i32 x_advance;
} Character;

typedef struct Font
{
    u32 tex_index;
    u32 width_atlas, height_atlas;
    u32 line_height;
    u32 pixels;
    u32 num_chars;
    Character* characters;
} Font;
