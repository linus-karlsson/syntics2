#pragma once

#include "defines.h"
#include "vulkan_types.h"

namespace synt {

typedef struct Region_Alloc Region_Alloc;

void init_render_testing(Region_Alloc* region, VkDevice device,
                         VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphic_queue,
                         const Swap_Chain_attrib& swap_chain, u32num_semaphores);

void render_testing_recreate(Region_Alloc* region,
                             const Application_State& app_state);

void render_testing_update(Region_Alloc* region, VkDevice device,
                           const Vec2& dimensions, u32semaphore_idx, f32dt);

void render_render_testing(VkCommandBuffer command_buffer, u32semaphore_idx);

void render_testing_destroy(VkDevice device, u32num_semaphores);

} // namespace synt
