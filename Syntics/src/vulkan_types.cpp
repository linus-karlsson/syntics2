#include "vulkan_types.h"

Swap_Chain_attrib::Swap_Chain_attrib()
    : img_views(0), images(0), framebuffers(0), num_images(0)
{
}
Descriptors::Descriptors() : desc_pool(VK_NULL_HANDLE), desc_count(0), desc_sets(0)
{
}

b8 operator==(const Vertex& f, const Vertex& s)
{
    return f.pos == s.pos && f.color == s.color && f.tex_coords == s.tex_coords &&
           f.tex_index == f.tex_coords;
}

b8 operator==(const MVP& f, const MVP& s)
{
    return f.model == s.model && f.view == s.view && f.proj == s.proj;
}

