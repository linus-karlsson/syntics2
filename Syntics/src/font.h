#include "defines.h"
#include "vulkan_types.h"

namespace synt {

typedef struct Region_Alloc Region_Alloc;

struct Character
{
    Character();
    int32& operator[](int i);
    int32 id;
    int32 x, y;
    int32 width, height;
    int32 x_offset, y_offset;
    int32 x_advance;
};

struct Font
{
    Font();

    uint32 tex_index;
    uint32 width_atlas, height_atlas;
    uint32 line_height;
    uint32 pixels;
    uint32 num_chars;
    Character* characters;
};

Font load_font_file(Region_Alloc* region, const char* file_path);

Font load_ftt_file(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, Texture** bitmaps, const char* file_path,
                   float scale);

Vec2 altas_coords_to_texidx(float x, float y, float atlas_width, float atlas_height);

uint32 text_3D(Font font, const char* text, Vec3 pos_first_letter, float size,
               float win_width, float win_height, Vertex** vertices);

uint32 text_2D(Font font, const char* text, Vec3 pos_first_letter, float size,
               Vertex** vertices);

uint32 text_2D_ttf(Font font, const char* text, Vec3 pos_first_letter, float size,
                   Vertex** vertices);
} // namespace synt
