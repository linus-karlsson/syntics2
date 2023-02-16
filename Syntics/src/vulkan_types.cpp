#include "vulkan_types.h"

Swap_Chain_attrib::Swap_Chain_attrib()
    : img_views(0), images(0), framebuffers(0), num_images(0)
{
}
Descriptors::Descriptors() : desc_pool(NULL), desc_count(0), desc_sets(0)
{
}

