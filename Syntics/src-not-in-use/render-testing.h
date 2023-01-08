#pragma once

#include "defines.h"
#include "vulkan_types.h"

namespace synt {

typedef struct Region_Alloc Region_Alloc;

void init_render_testing(Region_Alloc* region, VkDevice device,
                         VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphic_queue,
                         const Swap_Chain_attrib& swap_chain, uint32 num_semaphores);

void render_testing_recreate(Region_Alloc* region,
                             const Application_State& app_state);

void render_testing_update(Region_Alloc* region, VkDevice device,
                           const Vec2& dimensions, uint32 semaphore_idx, float dt);

void render_render_testing(VkCommandBuffer command_buffer, uint32 semaphore_idx);

void render_testing_destroy(VkDevice device, uint32 num_semaphores);

} // namespace synt
