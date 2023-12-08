#include "vulkan_types.h"


Graphic_Pipeline_Attrib gp_create(VkPrimitiveTopology topology, VkCullModeFlags cull_mode, VkPolygonMode poly_mode, u32 dynamic)
{
    Graphic_Pipeline_Attrib out = {0};
    out.topology = topology;
    out.cull_mode = cull_mode;
    out.poly_mode = poly_mode;
    out.dynamic = dynamic;
    out.line_width = 1.0f;
    return out;
}
