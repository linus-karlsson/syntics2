#include "render.h"
#include "defines.h"
#include "logging.h"
#include "region_alloc.h"
#include "buffers.h"
#include "event_system.h"
#include "swap_chain.h"
#include "file_reading.h"
#include "font.h"
#include "collision.h"
#include "render_util.h"
#include "gui.h"
#include "vulkan_types.h"
#include <string.h>
#include <math.h>

// #define CUSTOM_TOP_BAR
//
//  TODO: Probably will not have this
typedef struct Render_Task
{
    void (*draw_callback)(void* data, VkCommandBuffer command_buffer,
                          u32 semaphore_idx);
    void* data;
} Render_Task;

typedef struct Recreate_Task
{
    void (*rc_callback)(void* data, Region_Alloc* region,
                        const Application_State* app_state);
    void* data;
} Recreate_Task;

typedef struct Destroy_Task
{
    void (*rc_callback)(void* data, VkDevice device, u32 num_semaphores);
    void* data;
} Destroy_Task;

typedef struct Render_state
{
    VkFence* fences;
    VkSemaphore* image_semaphores;
    VkSemaphore* present_semaphores;

    VkCommandBuffer* command_buffers;

    Queues queues;

    // Topbar and other utilities
    Graphic_Pipline g_pipeline;
    MVP mvp;
    Font font;
    Rect2D* rects;

    Events* key_evt;
    Events* resize_evt;

    Texture* textures;

    Render_Task* render_tasks;
    Recreate_Task* rc_tasks;
    Destroy_Task* destroy_tasks;

} Render_state;

void init_platform_game(Region_Alloc* region, VkDevice device,
                        VkPhysicalDevice physical_device, VkCommandPool command_pool,
                        VkQueue graphic_queue, const Swap_Chain_attrib* swap_chain,
                        u32 num_semaphores);

void update_platform_game(Region_Alloc* region, VkDevice device, V2 dimensions,
                          u32 semaphore_idx, f32 dt);

static u32 NUM_SEMAPHORES = 2;
static u32 SEMAPHORE_INDEX = 0;
static Render_state render_state = { 0 };
static VkDevice device_handle = VK_NULL_HANDLE;

#define MAX_SPACE 100

void init_render_state(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device, VkCommandPool command_pool,
                       const Queue_Family_Indices* q_indices, u32 num_semaphores,
                       const Swap_Chain_attrib* swap_chain)
{
    device_handle = device;

    render_state.queues = queues;

    NUM_SEMAPHORES = num_semaphores;

    render_state.fences = region_mallocP(region, NUM_SEMAPHORES, VkFence);
    render_state.image_semaphores =
        region_mallocP(region, NUM_SEMAPHORES, VkSemaphore);
    render_state.present_semaphores =
        region_mallocP(region, NUM_SEMAPHORES, VkSemaphore);
    render_state.command_buffers =
        region_mallocP(region, NUM_SEMAPHORES, VkCommandBuffer);

    for (u32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        create_fence_semaphore(device, &render_state.fences[i],
                               &render_state.image_semaphores[i],
                               &render_state.present_semaphores[i]);

        allocate_commandbuffer(device, command_pool,
                               &render_state.command_buffers[i]);
    }

    render_state.render_tasks = dyn_arrayP(region, 10, Render_Task);
    render_state.rc_tasks = dyn_arrayP(region, 10, Recreate_Task);
    render_state.destroy_tasks = dyn_arrayP(region, 10, Destroy_Task);

    VkQueue graphic_queue = render_state.queues.graphic_queue;

#ifdef CUSTOM_TOP_BAR
    { // Graphic pipeline for topbar and other utilities;
        render_state.textures = dyn_arrayP(region, 3, Texture);
        // Default tex: 4 bytes big. 1x1 pixel white image
        create_texture(device, physical_device, command_pool, graphic_queue, false,
                       VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/default.png",
                       &render_state.textures[0]);
        get_head(render_state.textures)->size++;

        create_texture(device, physical_device, command_pool, graphic_queue, false,
                       VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/ArialWhiteSmall.png",
                       &render_state.textures[1]);
        get_head(render_state.textures)->size++;

        create_texture(device, physical_device, command_pool, graphic_queue, false,
                       VK_FORMAT_R8G8B8A8_SRGB, "Syntics/res/button.png",
                       &render_state.textures[2]);
        get_head(render_state.textures)->size++;

        create_graphics_pipeline(
            region, device, swap_chain.color_format, swap_chain.render_pass,
            swap_chain.sample_count, "Syntics/res/gui.vert.spv",
            "Syntics/res/gui.frag.spv", swap_chain.extent_2D.width,
            swap_chain.extent_2D.height, VK_CULL_MODE_BACK_BIT,
            size_arr(render_state.textures), NULL, &render_state.g_pipeline);

        init_graphics_pipeline(
            region, device, physical_device, command_pool, graphic_queue,
            MAX_SPACE * 4, NUM_SEMAPHORES, render_state.textures,
            size_arr(render_state.textures), render_state.g_pipeline);

        render_state.g_pipeline.idx_buffer.data =
            dyn_arrayP(region, MAX_SPACE * 6, u32);
        generate_indices(&render_state.g_pipeline.idx_buffer.data, 0, MAX_SPACE);
        render_state.g_pipeline.idx_buffer.size_bytes =
            capacity_arr(render_state.g_pipeline.idx_buffer.data) * sizeof(u32);
        create_index_buffer(device, physical_device, command_pool, graphic_queue,
                            &render_state.g_pipeline.idx_buffer);

        region_pop(region, capacity_arr(render_state.g_pipeline.idx_buffer.data),
                   u32, PERM_ARRAY);
        render_state.g_pipeline.idx_buffer.data = NULL;

        render_state.rects = dyn_arrayP(region, 10, Rect2D);

        render_state.font =
            load_font_file(region, "Syntics/res/ArialWhiteSmall.fnt");
        render_state.font.tex_index = 1.0f;

        render_state.mvp.model = mat4i(1.0f);
        render_state.mvp.view = mat4i(1.0f);
    }
#endif

    init_platform_game(region, device, physical_device, command_pool, graphic_queue,
                       swap_chain, NUM_SEMAPHORES);

    subscribe(&render_state.key_evt, EVT_KEY);
    subscribe(&render_state.resize_evt, EVT_RESIZE);
}

void create_fence_semaphore(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores)
{
    VkFenceCreateInfo fence_info = { 0 };
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphore_info = { 0 };
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_ASSERT(vkCreateFence(device, &fence_info, NULL, fence));
    VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, NULL, image_semaphores));
    VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, NULL, present_semaphores));
}

void draw_pipeline(void (*draw_callback)(void* data, VkCommandBuffer command_buffer,
                                         u32 semaphore_idx),
                   void* data)
{
    Render_Task task = { draw_callback, data };
    synt_push(render_state.render_tasks, task);
}

void subscribe_recreate_callback(
    void (*rc_callback)(void* data, Region_Alloc* region,
                        const Application_State* app_state),
    void* data)
{
    Recreate_Task task = { rc_callback, data };
    synt_push(render_state.rc_tasks, task);
}

void subscribe_destroy_callback(void (*destroy_callback)(void* data, VkDevice device,
                                                         u32 num_semaphores),
                                void* data)
{
    Destroy_Task task = { destroy_callback, data };
    synt_push(render_state.destroy_tasks, task);
}

static i32 clamp_i32(i32 value, i32 min, i32 high)
{
    if (value < min) return min;
    if (value > high) return high;
    return value;
}
static i32 clamp_i32_low(i32 value, i32 min)
{
    if (value < min) return min;
    return value;
}

static u32 hover_index = 0;
static u32 clicked_index = 0;

static b8 should_have_handle = false;

static b8 update_top_panel(u32* num_indices, V2 dimensions, f32 dt)
{
    Vertex_Buffer* vert = &render_state.g_pipeline.vert_buffer;
    get_head(render_state.rects)->size = 0;
    get_head(vert->data)->size = 0;
    u32 rect_index = 0;

    // V4 top_bar_color = V4(0.8f, 0.0f, 0.033f, 1.0f);
    V4 top_bar_color = v4f(0.03f, 0.03f, 0.03f, 1.0f);
    V4 buttons_color = v4f(1.0f, 1.0f, 1.0f, 1.0f);

    const b8 close_hover = should_have_handle = rect_index == hover_index;
    const b8 close_clicked = rect_index == clicked_index;

    V3 close_pos = v3f(dimensions.x - 12.0f, 10.0f, 0.0f);
    V2 close_size = v2f(12.0f, 1.0f);
    quad_r(&vert->data, num_indices, close_pos, close_size, buttons_color, 0.0f,
           radians(45.0f));
    quad_r(&vert->data, num_indices, close_pos, close_size, buttons_color, 0.0f,
           radians(-45.0f));

    V4 rect_color = top_bar_color;
    f32 hover_multiplier = 1.6f;

    if (close_hover)
    {
        rect_color = v4f(0.2f, 0.0f, 0.033f, 1.0f);
    }
    V3 rect_pos = v3f(dimensions.x - 25.0f, 0.0f, 0.0f);
    V2 rect_size = v2f(25.0f, 20.0f);
    synt_push(render_state.rects,
              quad_d1(&vert->data, num_indices, rect_pos, rect_size, rect_color));

    rect_index++;

    const b8 max_hover = rect_index == hover_index;
    const b8 max_clicked = rect_index == clicked_index;

    rect_color = top_bar_color;

    V3 top_left = v3f(close_pos.x - 30.0f, close_pos.y - 5.5f, close_pos.z);
    add_border_d1(&vert->data, num_indices, buttons_color, top_left, v2i(10.0f),
                  1.0f);

    rect_color = top_bar_color;
    if (max_hover)
    {
        v4_s_multi_equal(&rect_color, hover_multiplier);
    }
    rect_pos.x -= rect_size.x;
    synt_push(render_state.rects,
              quad_d1(&vert->data, num_indices, rect_pos, rect_size, rect_color));
    rect_index++;

    const b8 minimize_hover = rect_index == hover_index;
    const b8 minimize_clicked = rect_index == clicked_index;

    close_pos.x -= 57.0f;
    close_pos.y -= 1.0f;
    quad_d1(&vert->data, num_indices, close_pos, close_size, buttons_color);

    rect_color = top_bar_color;
    if (minimize_hover)
    {
        v4_s_multi_equal(&rect_color, hover_multiplier);
    }
    rect_pos.x -= rect_size.x;
    synt_push(render_state.rects,
              quad_d1(&vert->data, num_indices, rect_pos, rect_size, rect_color));
    rect_index++;

    const b8 topbar_clicked = rect_index == clicked_index;
    const b8 top_bar_hover = rect_index == hover_index;

    synt_push(render_state.rects,
              quad_s_gradiant_d1(&vert->data, num_indices, v3i(0.0f),
                                 v2f(dimensions.x, 20.0f), top_bar_color));
    synt_back(render_state.rects)->size.x -= 100.0f;
    rect_index++;

    // Border
    add_border_d1(&vert->data, num_indices, top_bar_color, v3i(0.0f), dimensions,
                  3.0f);

    b8 presist_hold = is_any_button_pressed();

    if (close_clicked)
    {
        return false;
    }
    if (max_clicked)
    {
        sy_toggle_maximize();
        set_button_unpressed();
    }
    static i32 presist_offset_x = 0;
    static i32 presist_offset_y = 0;
    static b8 top_bar_hold = false;
    if (topbar_clicked)
    {
        i32 x, y;
        i16 m_x, m_y;
        if (is_maximized())
        {
            sy_toggle_maximize();

            u16 w, h;
            get_window_size(&w, &h);
            get_screen_pos(&x, &y);

            i32 half_w = (i32)w / 2;
            m_x = (i16)x;
            m_y = (i16)y;
            m_x -= (i16)(x = x - half_w);
            m_y -= (i16)(y -= 10);

            sy_move_window(clamp_i32_low(x, 0), clamp_i32_low(y, 0), (i32)w, (i32)h);

            x += half_w;
            y += 10;
        }
        else
        {
            get_screen_pos(&x, &y);
            get_pos(&m_x, &m_y);
        }

        m_x = (i16)x - m_x;
        m_y = (i16)y - m_y;

        top_bar_hold = true;
        presist_offset_x = x - (i32)m_x;
        presist_offset_y = y - (i32)m_y;
    }
    if (top_bar_hold && presist_hold)
    {
        i32 x, y;
        get_screen_pos(&x, &y);
        u16 w, h;
        get_window_size(&w, &h);

        x = x - presist_offset_x;
        y = y - presist_offset_y;

        sy_move_window(x, y, (i32)w, (i32)h);
    }
    else if (close_hover || max_hover || minimize_hover)
    {
        change_cursor(SYNT_HAND_CURSOR);
    }
    else if (top_bar_hover)
    {
        if (top_bar_hold)
        {
            i32 x, y;
            get_screen_pos(&x, &y);
            if (y < 1.0f)
            {
                sy_toggle_maximize();
            }
            top_bar_hold = false;
        }
        change_cursor(SYNT_NORMAL_CURSOR);
    }

    *num_indices *= 6;

    map_copy_mem(device_handle, &render_state.g_pipeline.vert_buffer.buffer_memory,
                 render_state.g_pipeline.vert_buffer.size_bytes,
                 render_state.g_pipeline.vert_buffer.data);
    return true;
}

void render(Region_Alloc* region, Application_State* app_state, f32 dt)
{
    f32 swap_chain_width = (f32)app_state->swap_chain.extent_2D.width;
    f32 swap_chain_height = (f32)app_state->swap_chain.extent_2D.height;

    vkWaitForFences(device_handle, 1, &render_state.fences[SEMAPHORE_INDEX], VK_TRUE,
                    UINT64_MAX);

    u32 image_index = 0;
    VkResult result = vkAcquireNextImageKHR(
        device_handle, app_state->swap_chain.swap_chain, UINT64_MAX,
        render_state.image_semaphores[SEMAPHORE_INDEX], VK_NULL_HANDLE,
        &image_index);

    vkResetFences(device_handle, 1, &render_state.fences[SEMAPHORE_INDEX]);

    b8 hit = false;
#ifdef CUSTOM_TOP_BAR
    if (!is_fullscreen())
    {
        app_state.running = update_top_panel(
            &num_indices, V2(swap_chain_width, swap_chain_height), dt);

        i16 x, y;
        get_pos(&x, &y);
        V2 mouse_pos = V2((f32)x, (f32)y);

        static b8 first_clicked = true;
        const b8 button_clicked = is_any_button_clicked(&first_clicked);

        hover_index = -1;
        clicked_index = -1;

        for_range(i, size_arr(render_state.rects))
        {
            if (point_in_rect(mouse_pos, render_state.rects[i]))
            {
                hit = true;
                hover_index = i;
                if (button_clicked)
                {
                    clicked_index = i;
                }
                break;
            }
        }
    }

    render_state.mvp.proj =
        ortho(0, 0, swap_chain_width, swap_chain_height, -1.0f, 1.0f);
    update_uniform_buffers(app_state.device,
                           render_state.g_pipeline.uniform_buffers[SEMAPHORE_INDEX],
                           &render_state.mvp, sizeof(render_state.mvp));

#endif
    update_platform_game(region, device_handle,
                         v2f(swap_chain_width, swap_chain_height), SEMAPHORE_INDEX,
                         dt);
    if (!hit && !gui_focus())
    {
        change_cursor(SYNT_NORMAL_CURSOR);
    }

    begin_render_pass(render_state.command_buffers[SEMAPHORE_INDEX],
                      app_state->swap_chain.render_pass,
                      app_state->swap_chain.framebuffers[image_index],
                      &app_state->swap_chain.extent_2D);
    {
#ifdef CUSTOM_TOP_BAR
        if (!is_fullscreen())
        {
            bind_and_draw_graphics_pipline(
                render_state.command_buffers[SEMAPHORE_INDEX],
                render_state.g_pipeline.descriptors.desc_sets[SEMAPHORE_INDEX], 0,
                num_indices, render_state.g_pipeline);
        }
#endif
        u32 size = size_arr(render_state.render_tasks);
        for_range(i, size)
        {
            Render_Task* t = &render_state.render_tasks[i];
            t->draw_callback(t->data, render_state.command_buffers[SEMAPHORE_INDEX],
                             SEMAPHORE_INDEX);
        }
    }
    end_render_pass(render_state.command_buffers[SEMAPHORE_INDEX]);

    get_head(render_state.render_tasks)->size = 0;

    submit_and_present(render_state.queues.graphic_queue,
                       render_state.queues.present_queue,
                       render_state.image_semaphores[SEMAPHORE_INDEX],
                       render_state.present_semaphores[SEMAPHORE_INDEX],
                       render_state.fences[SEMAPHORE_INDEX],
                       render_state.command_buffers[SEMAPHORE_INDEX],
                       app_state->swap_chain.swap_chain, image_index);

    if (render_state.resize_evt->resize_evt.is_resized ||
        result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        Resize_Evt* e = &render_state.resize_evt->resize_evt;
        e->is_resized = false;
        recreate_swapchain(region, app_state, e->width, e->height,
                           /*size_arr(render_state.textures)*/ 0);

#ifdef CUSTOM_TOP_BAR
        recreate_graphic_pipline(region, device_handle, app_state.swap_chain,
                                 "Syntics/res/gui.vert.spv",
                                 "Syntics/res/gui.frag.spv", render_state.g_pipeline,
                                 size_arr(render_state.textures), NULL);
#endif

        u32 size = size_arr(render_state.rc_tasks);
        for_range(i, size)
        {
            Recreate_Task* t = &render_state.rc_tasks[i];
            t->rc_callback(t->data, region, app_state);
        }
    }

    SEMAPHORE_INDEX++;
    SEMAPHORE_INDEX %= NUM_SEMAPHORES;
}

void submit_and_present(VkQueue graphic_queue, VkQueue present_queue,
                        VkSemaphore image_semaphore, VkSemaphore present_semaphore,
                        VkFence fence, VkCommandBuffer command_buffer,
                        VkSwapchainKHR swap_chain, u32 image_index)
{

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info = { 0 };
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_semaphore;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &present_semaphore;

    VK_ASSERT(vkQueueSubmit(graphic_queue, 1, &submit_info, fence));

    VkPresentInfoKHR present_info = { 0 };
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &present_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swap_chain;
    present_info.pImageIndices = &image_index;

    vkQueuePresentKHR(present_queue, &present_info);
}

void destroy_render_state()
{
    for (u32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        vkDestroyFence(device_handle, render_state.fences[i], NULL);
        vkDestroySemaphore(device_handle, render_state.image_semaphores[i], NULL);
        vkDestroySemaphore(device_handle, render_state.present_semaphores[i], NULL);
    }

#ifdef CUSTOM_TOP_BAR
    destroy_graphic_pipeline(device_handle, NUM_SEMAPHORES, render_state.g_pipeline);

    for (u32 i = 0; i < size_arr(render_state.textures); i++)
    {
        destroy_texture(device_handle, render_state.textures[i]);
    }
#endif

    u32 size = size_arr(render_state.destroy_tasks);
    for_range(i, size)
    {
        Destroy_Task* d = &render_state.destroy_tasks[i];
        d->rc_callback(d->data, device_handle, NUM_SEMAPHORES);
    }
}

