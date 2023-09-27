#ifndef SY_INCLUDES // only for clangd
#include "syntics.h"
#endif

#define DEFAULT_TEXTURE_NOTE 0

u32 text_gen(Notebook* note, V3 pos, f32 scale, f32 line_height,
             const char* text)
{

    Vertex_Buffer* vert = &note->vert_idx.vert;
    Texture texture = note->textures[1];
    f32 start_x = pos.x;
    u32 count = 0;
    for (; *text; text++, count++)
    {
        char current_char = *text;
        if (current_char == '\n')
        {
            pos.y += line_height * scale;
            pos.x = start_x;
        }
        stbtt_bakedchar c = note->cdata[(*text) - 32];

        V4 tex_coords =
            v4f((f32)c.x0 / texture.width, (f32)c.y0 / texture.height,
                (f32)c.x1 / texture.width, (f32)c.y1 / texture.height);

        V2 size = v2f((f32)(c.x1 - c.x0), (f32)(c.y1 - c.y0));
        size = v2_s_multi(size, scale);

        V3 curr_pos = pos;
        curr_pos.x += c.xoff * scale;
        curr_pos.y += c.yoff * scale;
        quad_co(&vert->array, NULL, curr_pos, size, v4f(1.0f, 1.0f, 1.0f, 1.0f),
                tex_coords, 1.0f);

        pos.x += c.xadvance * scale;
    }

    return count;
}

void notebook_init(Region_Alloc* region, VkDevice device,
                   VkPhysicalDevice physical_device, VkCommandPool command_pool,
                   VkQueue graphic_queue, const Swap_Chain_Attrib* swap_chain,
                   const Platform* platform, Render_State* render_state,
                   u32 num_semaphores, Notebook* notebook)
{
    stack_begin_scope(notebook_init_stack);
    const char* paths[] = {
        [DEFAULT_TEXTURE_NOTE] = "Syntics/res/default.png",
    };
    u32 num_text = 1;

    notebook->textures = region_array(region, num_text + 1, Texture);

    textures_path_create(device, physical_device, command_pool, graphic_queue,
                         false, num_text, paths, notebook->textures);

    array_head(notebook->textures)->size = num_text;

    char* ttf_file_path = path_extend_d1("Syntics/res/ubuntu/Ubuntu-M.ttf");
    File_Attrib ttf_file = { 0 };
    file_read(&ttf_file, stack_get(), ttf_file_path);
    const f32 pixel_height = 32.0f;
#if 0
    stbtt_fontinfo font = { 0 };
    stbtt_InitFont(&font, ttf_file.buffer,
                   stbtt_GetFontOffsetForIndex(ttf_file.buffer, 0));

    i32 width = 0;
    i32 height = 0;
    u8* bitmap = stbtt_GetCodepointBitmap(
        &font, 0, stbtt_ScaleForPixelHeight(&font, pixel_height), 'A', &width,
        &height, NULL, NULL);
    assert(bitmap);
#else
    i32 width = 512;
    i32 height = 512;
    u8 bitmap[width * height];
    stbtt_BakeFontBitmap(ttf_file.buffer, 0, pixel_height, bitmap, 512, 512, 32,
                         96, notebook->cdata);
#endif

    Texture text = { 0 };
    text.mip_map_lvl = 1;
    text.width = (u32)width;
    text.height = (u32)height;

#if 0
    text.size_bytes = (u32)(width * height * 4);
    u8* buffer = stack_array0((u32)text.size_bytes, u8);
    u8* source = bitmap;
    u32* destination = (u32*)buffer;
    for (i32 i = 0; i < height; i++)
    {
        for (i32 j = 0; j < width; j++)
        {
            u8 alpha = *source++;
            *destination++ = 0x00FFFFFF | (((u32)alpha) << 24); 
        }
    }
    texture_buffer_create(device, physical_device, command_pool, graphic_queue,
                          VK_FORMAT_R8G8B8A8_SRGB, buffer, &text);
#else
    text.size_bytes = (u32)(width * height);
    texture_buffer_create(device, physical_device, command_pool, graphic_queue,
                          VK_FORMAT_R8_SRGB, bitmap, &text);
#endif
    array_push(notebook->textures, text);

    // stbtt_FreeBitmap(bitmap, NULL);

    Vertex_Buffer* vert = &notebook->vert_idx.vert;
    Index_Buffer* idx = &notebook->vert_idx.idx;

    const u32 quads = 1000;
    const u32 vertices = 4 * quads;
    const u32 indices = 6 * quads;
    vert->array = vertex_array_create(stack_get(), vertices);
    idx->array = u32_array_create(stack_get(), indices);

    u32 count = text_gen(notebook, v3f(10.0f, 100.0f, 0.0f), 0.6f, pixel_height,
                         "Hello my name\nis this");
    printf("%u\n", count);

    indices_generate(&idx->array, 0, quads);

    vertex_index_buffer_create_default1(device, physical_device, command_pool,
                                        graphic_queue, VERTEX_INDEX_LOCAL_LOCAL,
                                        &notebook->vert_idx);

    descriptor_set_layout_create(device, array_size(notebook->textures),
                                 &notebook->descriptor_set_layout);

    pipeline_layout_create(device, notebook->descriptor_set_layout,
                           &notebook->pipeline_layout);

    uniforms_descriptors_init(
        region, device, physical_device, &notebook->uniform_buffers,
        &notebook->descriptors, notebook->descriptor_set_layout, num_semaphores,
        notebook->textures, array_size(notebook->textures));

    { // Triangle list
        Graphic_Pipeline_Attrib g_p_info = gp_default2(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT);
        graphics_pipeline_create_deluxe(
            device, notebook->pipeline_layout, &g_p_info,
            "Syntics/res/shaders/spv/notebook.vert.spv",
            "Syntics/res/shaders/spv/notebook.frag.spv", swap_chain,
            &notebook->triangle_list_pipeline);
    }
    stack_end_scope(notebook_init_stack);
}

void notebook_copy_buffer(void* data, VkCommandBuffer command_buffer,
                          u32 semaphore_idx)
{
    Notebook* note = (Notebook*)data;
    assert(note);

    data_buffer_copy(&note->uniform_buffers[semaphore_idx], &note->vp,
                     sizeof(note->vp));
}

void notebook_render(void* data, VkCommandBuffer command_buffer,
                     u32 semaphore_idx)
{
    Notebook* note = (Notebook*)data;
    // NOTE: REMEMBER TO COPY UNIFORM BUFFERS

    // NOTE: same for every draw call at the moment
    VkViewport view_port = { 0 };
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = note->dimensions.width;
    view_port.height = note->dimensions.height;
    view_port.maxDepth = 1.0f;

    VkRect2D scissor_internal = { { (i32)view_port.x, (i32)view_port.y },
                                  { (u32)view_port.width,
                                    (u32)view_port.height } };
    vkCmdSetViewport(command_buffer, 0, 1, &view_port);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor_internal);

    vkCmdBindDescriptorSets(
        command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, note->pipeline_layout,
        0, 1, &note->descriptors.desc_sets[semaphore_idx], 0, NULL);

    vertex_index_buffer1_bind(command_buffer, &note->vert_idx);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      note->triangle_list_pipeline);

    Push_Constant global_constant;
    global_constant.model = m4i(1.0f);
    global_constant.normal = m4i(1.0f);
    push_constant(command_buffer, note->pipeline_layout, &global_constant,
                  sizeof(global_constant));

    draw(command_buffer, 0, note->vert_idx.idx.array.size);
}

void notebook_update_gui(Notebook* note, Gui_Context* gui_ctx, f32 dt,
                         V2 dimensions)
{
    Ui_Window* win = window_begin(gui_ctx, array_val(note->win_handles, 0),
                                  "Terminal", v2f(500.0f, 100.0f));
    {
        terminal_add(gui_ctx, terminal_ptr_get(), win, 250.0f, 200.0f);
    }
    window_end(&win);
}

void notebook_update(Notebook* note, Gui_Context* gui_ctx,
                     Application_State* app_state, Render_Task* copy_tasks,
                     Render_Task* render_tasks, V2 dimensions,
                     u32 semaphore_idx, f32 dt)
{
    note->dimensions = dimensions;
    note->vp.view = m4i(1.0f);
    note->vp.proj = ortho(0.0f, dimensions.x, 0.0f, dimensions.y, -1.0f, 1.0f);
    notebook_update_gui(note, gui_ctx, dt, dimensions);

    Render_Task task = { .callback = notebook_render, .data = note };
    array_push(render_tasks, task);
    task = (Render_Task){ .callback = notebook_copy_buffer, .data = note };
    array_push(copy_tasks, task);
}
