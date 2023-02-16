#pragma once

#include "vulkan_internal_api.h"

typedef struct Region_Alloc Region_Alloc;
void init_vulkan(Region_Alloc* region, Application_State* app_state, u32 width,
                 u32 height);
void destroy_vulkan();

