#pragma once

#include "defines.h"
#include "vulkan_types.h"

namespace synt {

typedef struct Region_Alloc Region_Alloc;

void gui_init(Region_Alloc* region, VkDevice device,
              VkPhysicalDevice physical_device, VkCommandPool command_pool,
              VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
              uint32 num_semaphores);

void gui_render(VkCommandBuffer command_buffer, uint32 semaphore_idx);

void gui_update(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                uint32 semaphore_idx, float delta);

void gridd_begin(uint32 x, uint32 y);

void gridd_end();

uint32 add_button(const char* text);

uint32 add_input_float(float& input);

void destroy_gui(VkDevice device, uint32 num_semaphores);

bool gui_focus();

} // namespace synt
