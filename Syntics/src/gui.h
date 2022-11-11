#pragma once

#include "defines.h"
#include "vulkan_types.h"

namespace synt {

typedef struct Region_Alloc Region_Alloc;

void gui_init(Region_Alloc* region, VkDevice device,
              VkPhysicalDevice physical_device, VkCommandPool command_pool,
              VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain);

void gui_render(VkCommandBuffer command_buffer, uint32 semaphore_idx);

void gui_update(float delta);

void gridd_begin(uint32 x, uint32 y);

void gridd_end();

void add_button();

} // namespace synt
