#pragma once

#include "defines.h"
#include "vulkan_types.h"

namespace synt {

typedef struct Region_Alloc Region_Alloc;

void jail_init(Region_Alloc* region, VkDevice device,
               VkPhysicalDevice physical_device, VkCommandPool command_pool,
               VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
               u32num_semaphores);

void jail_recreate(Region_Alloc* region, const Application_State& app_state);

void jail_update(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                 u32semaphore_idx, f32dt);

void jail_render(VkCommandBuffer command_buffer, u32semaphore_idx);

void jail_destroy(VkDevice device, u32num_semaphores);

} // namespace synt
