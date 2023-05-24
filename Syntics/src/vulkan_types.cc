#include "vulkan_types.h"

Graphic_Pipeline gp_create(VkPrimitiveTopology topology, VkCullModeFlags cull_mode, VkPolygonMode poly_mode, u32 dynamic)
{
    Graphic_Pipeline out = {0};
    out.topology = topology;
    out.cull_mode = cull_mode;
    out.poly_mode = poly_mode;
    out.dynamic = dynamic;
    out.line_width = 1.0f;
    return out;
}
