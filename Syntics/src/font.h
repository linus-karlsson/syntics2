#include "defines.h"
#include "math/matrix.h"

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

Font load_font_file(Region_Alloc* region, const char* file_path);

#if 0
Font load_ftt_file(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, Texture** bitmaps, const char* file_path,
                   f32 scale);
#endif

Vec2 altas_coords_to_texidx(f32 x, f32 y, f32 atlas_width, f32 atlas_height);

u32 text_3D(Font font, const char* text, Vec3 pos_first_letter, f32 size,
            f32 win_width, f32 win_height, Vertex** vertices);

u32 text_2D(Font font, f32 y_origin, const char* text, u32 text_len,
            Vec3 pos_first_letter, Vec4 color, f32 size, u32* new_lines,
            float* x_adv, Vertex** vertices);

u32 text_2D_ttf(Font font, const char* text, Vec3 pos_first_letter, f32 size,
                Vertex** vertices);
