#include "defines.h"
#include <unordered_map>

namespace synt {

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

    uint32_t width_atlas, height_atlas;
    std::unordered_map<int32, Character> characters;
};

Font load_font_file(const char* file_path);

uint32_t text(Font font, const char* text);

} // namespace synt
