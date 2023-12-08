#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

void vulkan_init(Region_Alloc* region, Instance_State* instance_state,
                 Application_State* app_state, Render_State** render_state, u32 width, u32 height);
void vulkan_destroy(Application_State* app_state, Render_State* render_state);
