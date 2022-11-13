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

void gui_update_begin(Region_Alloc* region, VkDevice device, const Vec2& dimensions,
                      uint32 semaphore_idx, float delta);

void gui_update_end(Region_Alloc* region, VkDevice device);

void gridd_begin(uint32 x, uint32 y);

void gridd_end();

bool add_button(const char* text);

bool add_input_float(float& input);

void destroy_gui(VkDevice device, uint32 num_semaphores);

bool gui_focus();

void set_data(void* data);

} // namespace synt
