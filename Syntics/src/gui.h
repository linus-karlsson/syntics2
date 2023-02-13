#pragma once

#include "defines.h"
#include "vulkan_types.h"

typedef struct Region_Alloc Region_Alloc;

void gui_init(Region_Alloc* region, VkDevice device,
              VkPhysicalDevice physical_device, VkCommandPool command_pool,
              VkQueue graphic_queue, const Swap_Chain_attrib& swap_chain,
              u32 num_semaphores);

void gui_terminal_init(Region_Alloc* region);

void gui_render(VkCommandBuffer command_buffer, u32 semaphore_idx);

void gui_recreate(Region_Alloc* region);

void gui_update_begin(Region_Alloc* region, const Vec2& dimensions,
                      u32 semaphore_idx, f32 delta, f32 translucentcy = 1.0f);

void gui_update_end();

void gridd_begin(u32 x, u32 y);

void gridd_end();

void back_bord_begin(const char* title, const Vec2& pos);

void back_bord_end();

b8 add_button(const char* text);

b8 add_input_float(f32& input, f32 min, f32 max);

b8 add_input_text(char** ptr_to_text, uint32* size);

void add_text(const char* text);

void add_terminal(f32 width, f32 height);

void add_graph(f32 value, const char* y_title, f32 y_max, f32 y_min, f32 sample_rate,
               f32 dt);

void destroy_gui(VkDevice device, u32 num_semaphores);

b8 gui_focus();

