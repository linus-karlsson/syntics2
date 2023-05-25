#pragma once

#include "vulkan_internal_api.h"

namespace sygui {

typedef void* Window_Handle;

void init(Region_Alloc* region, VkDevice device, VkPhysicalDevice physical_device,
          VkCommandPool command_pool, VkQueue graphic_queue,
          const Swap_Chain_attrib* swap_chain, u32 num_semaphores, b32 use_save);

void init_terminal(Region_Alloc* region);

// void gui_render(VkCommandBuffer command_buffer, u32 semaphore_idx);

void recreate(Region_Alloc* region);

void begin_update(Region_Alloc* region, V2 dimensions, u32 semaphore_idx, f32 delta,
                  f32 translucentcy);

void end_update(void);

Window_Handle create_window();

void free_window(Window_Handle handle);

void begin_pane(Window_Handle handle, const char* title, V2 pos);

void end_pane(void);

void begin_gridd(u32 x, u32 y);

void end_gridd(void);

b8 add_button(const char* text);

#define add_input_float_d(input, min, max)                                      \
    add_input_float(input, min, max, (max - min) * 0.4f)
b8 add_input_float(f32* input, f32 min, f32 max, f32 speed);

b8 add_input_text(char* ptr_to_text, uint32* size);

void add_text(const char* text);

void add_terminal(f32 width, f32 height);

void add_graph(f32 value, const char* y_title, f32 y_max, f32 y_min, f32 sample_rate,
               f32 dt);

void show_edit_entity(Dynamic_Entity_2D* e, char* name);

void show_entity(Dynamic_Entity_2D* e, char* name);

void entity_watch_window(void);

void destroy(VkDevice device, u32 num_semaphores);

b8 is_focus(void);
} // namespace gui

