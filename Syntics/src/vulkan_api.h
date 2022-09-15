#pragma once

#include "vulkan_types.h"

namespace synt {
typedef struct Region_Alloc Region_Alloc;
void init_vulkan(Region_Alloc* region, Application_State* app_state, uint32 height,
                 uint32 width);
void destroy_vulkan();
} // namespace synt

