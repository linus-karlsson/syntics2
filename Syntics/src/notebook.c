#ifndef SY_INCLUDES // only for clangd
#include "syntics.h"
#endif

void notebook_init(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
                   const Platform* platform, Render_State* render_state,
                   u32 num_semaphores, Notebook* notebook)
{
    const char* paths[] = {
        [DEFAULT_TEXTURE_GAME] = "Syntics/res/default.png",
        [OBJ_TEXTURE_GAME] = "Syntics/res/Purisa.png",
    };
    u32 num_text = sy_SIZE(paths);
    notebook->textures = region_array(region, num_text, Texture);

    textures_path_create(device, physical_device, command_pool, graphic_queue,
                         false, num_text, paths, notebook->textures);

    array_head(notebook->textures)->size = num_text;

    descriptor_set_layout_create(device, num_text,
                                 &notebook->descriptor_set_layout);
    pipeline_layout_create(device, notebook->descriptor_set_layout,
                           &notebook->pipeline_layout);

    uniforms_descriptors_init(
        region, device, physical_device, &notebook->uniform_buffers,
        &notebook->descriptors, notebook->descriptor_set_layout, num_semaphores,
        notebook->textures, num_text);

    { // Triangle list
        Graphic_Pipeline_Attrib g_p_info = gp_default2(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT);
        graphics_pipeline_create_deluxe(
            device, notebook->pipeline_layout, &g_p_info,
            "Syntics/res/shaders/spv/notebook.vert.spv",
            "Syntics/res/shaders/spv/notebook.frag.spv", swap_chain,
            &notebook->triangle_list_pipeline);
    }
}

void notebook_render(void* data, VkCommandBuffer command_buffer,
                     u32 semaphore_idx)
{
    Notebook* frame = (Notebook*)data;
    // NOTE: REMEMBER TO COPY UNIFORM BUFFERS

    // NOTE: same for every draw call at the moment
    VkViewport view_port = { 0 };
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = frame->dimensions.width;
    view_port.height = frame->dimensions.height;
    view_port.maxDepth = 1.0f;

    VkRect2D scissor_internal = { { (i32)view_port.x, (i32)view_port.y },
                                  { (u32)view_port.width,
                                    (u32)view_port.height } };
    vkCmdSetViewport(command_buffer, 0, 1, &view_port);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor_internal);

    vkCmdBindDescriptorSets(
        command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, frame->pipeline_layout,
        0, 1, &frame->descriptors.desc_sets[semaphore_idx], 0, NULL);
}

void notebook_update_gui(Notebook* note, Gui_Context* gui_ctx, f32 dt,
                     V2 dimensions)
{
    Ui_Window* win = window_begin(gui_ctx, array_val(note->win_handles, 0), "Terminal",
                       v2f(500.0f, 100.0f));
    {
        terminal_add(gui_ctx, terminal_ptr_get(), win, 250.0f, 200.0f);
    }
    window_end(&win);
}

void notebook_update(Notebook* note, Gui_Context* gui_ctx,
                     Application_State* app_state, V2 dimensions,
                     u32 semaphore_idx, f32 dt)
{
   notebook_update_gui(note, gui_ctx, dt, dimensions);
}
