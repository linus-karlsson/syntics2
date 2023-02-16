#pragma once

#include "defines.h"
#include "vulkan_types.h"

typedef struct Region_Alloc Region_Alloc;

void init_terrain(Region_Alloc* region, VkDevice device,
                  VkPhysicalDevice physical_device, VkCommandPool command_pool,
                  VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
                  u32 semaphores);

void recreate_terrain(Region_Alloc* region, const Application_State& app_state);

void update_terrain(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                    u32 semaphore_idx, f32 dt);

void render_terrain(VkCommandBuffer command_buffer, u32 semaphore_idx);

void destroy_terrain(VkDevice device, u32 semaphores);

