#pragma once

#include "vulkan_internal_api.h"

void init_vulkan(Region_Alloc* region, Application_State* app_state, u32 width,
                 u32 height);
void destroy_vulkan();

