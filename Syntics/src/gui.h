#pragma once

#include "defines.h"
#include "vulkan_types.h"

typedef struct Region_Alloc Region_Alloc;

void gui_init(Region_Alloc* region, VkDevice device,
              VkPhysicalDevice physical_device, VkCommandPool command_pool,
              VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
              uint32 num_semaphores);

void gui_render(VkCommandBuffer command_buffer, uint32 semaphore_idx);

void gui_recreate(Region_Alloc* region);

void gui_update_begin(Region_Alloc* region, const Vec2& dimensions,
                      uint32 semaphore_idx, float delta);

void gui_update_end();

void gridd_begin(uint32 x, uint32 y);

void gridd_end();

void back_bord_begin(const char* title, const Vec2& pos);

void back_bord_end();

bool add_button(const char* text);

bool add_input_float(float& input, float min, float max);

void add_text(const char* text);

void add_terminal(float width, float height);

void destroy_gui(VkDevice device, uint32 num_semaphores);

bool gui_focus();

