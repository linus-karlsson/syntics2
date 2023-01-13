#pragma once

#include "vulkan_types.h"

typedef struct Region_Alloc Region_Alloc;
void init_vulkan(Region_Alloc* region, Application_State* app_state, uint32 width,
                 uint32 height);
void destroy_vulkan();

