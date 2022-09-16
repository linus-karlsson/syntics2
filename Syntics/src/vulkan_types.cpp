#include "vulkan_types.h"

namespace synt {

Swap_Chain_attrib::Swap_Chain_attrib()
    : img_views(0), images(0), framebuffers(0), num_images(0)
{
}
Descriptors::Descriptors() : desc_pool(VK_NULL_HANDLE), desc_sets(0), desc_count(0) {}

} // namespace synt
