#include "defines.h"
#include <unordered_map>

namespace synt {

typedef struct Vertex Vertex;
typedef struct Vec3 Vec3;
typedef struct Vec2 Vec2;
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
    std::unordered_map<int32, Character> characters;
};

Font load_font_file(const char* file_path);

Vec2 altas_coords_to_texidx(float x, float y, float atlas_width,
                            float atlas_height);

uint32 text(Region_Alloc* region, Font font, const char* text,
            Vec3 pos_first_letter, uint16 width, uint16 height,
            Vertex** vertices);

} // namespace synt
