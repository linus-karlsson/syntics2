#include "vulkan_types.h"

namespace synt {

Swap_Chain_attrib::Swap_Chain_attrib()
    : img_views(0), images(0), framebuffers(0), num_images(0)
{
}
Descriptors::Descriptors() : desc_pool(VK_NULL_HANDLE), desc_sets(0), desc_count(0) {}

bool operator==(const Vertex& f, const Vertex& s)
{
    return f.pos == s.pos && f.color == s.color && f.tex_coords == s.tex_coords &&
           f.tex_index == f.tex_coords;
}

bool operator==(const MVP& f, const MVP& s)
{
    return f.model == s.model && f.view == s.view && f.proj == s.proj;
}

} // namespace synt
