#include "render.h"
#include "region_alloc.h"
#include "buffers.h"
#include "event_system.h"
#include "swap_chain.h"
#include "file_reading.h"
#include "gui.h"
#include "platform_game.h"
#include "terrain.h"
#include "font.h"
#include <string.h>
#include <math.h>

#define GUI_ON
#define GAME_ON

typedef struct Render_state
{

    VkFence* fences;
    VkSemaphore* image_semaphores;
    VkSemaphore* present_semaphores;

    VkCommandBuffer* command_buffers;

    Queues queues;

    // Topbar and other utilities
    Graphic_Pipline g_pipline;
    MVP mvp;
    Font font;

    Events* key_evt;
    Events* resize_evt;

    Texture* textures;

} Render_state;

static u32 NUM_SEMAPHORES = 2;
static u32 SEMAPHORE_INDEX = 0;
static Render_state render_state = {};
static VkDevice device_handle = VK_NULL_HANDLE;

static int32 maxi32(int32 f, int32 s)
{
    return (f > s) ? f : s;
}

static inline Vec2 mouse_pos_to_pos(const Vec2& mouse_pos, const Vec2& window_size)
{
    // Pos from top left corner (0, 0)
    static const f32 x_start = -1.0f;
    static const f32 y_start = -1.0f;

    return Vec2((x_start + ((mouse_pos.x * 2) / window_size.x)),
                (y_start + ((mouse_pos.y * 2) / window_size.y)));
}

#if 0
Vertex verts[4 * 6] = {};
verts[0].pos        = { -0.5f, -0.5f, -10.9f };
verts[0].tex_coords = { 0.0f, 0.0f };

verts[1].pos        = { -0.5f, 0.5f, -10.9f };
verts[1].tex_coords = { 0.0f, 1.0f };

verts[2].pos        = { 0.5f, 0.5f, -10.9f };
verts[2].tex_coords = { 1.0f, 1.0f };

verts[3].pos        = { 0.5f, -0.5f, -10.9f };
verts[3].tex_coords = { 1.0f, 0.0f };

for (u32i = 0; i < 4; i++)
{
    synt_push(render_state.graphic_piplines[UI_PIPELINE].vert_buffer.data,
              verts[i]);
}

#endif

#define MAX_SPACE 100

void init_render_state(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device, VkCommandPool command_pool,
                       const Queue_Family_Indices& q_indices, u32 num_semaphores,
                       const Swap_Chain_attrib& swap_chain)
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

    VkQueue graphic_queue = render_state.queues.graphic_queue;

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
            size_arr(render_state.textures), NULL, &render_state.g_pipline);

        init_graphics_pipeline(
            region, device, physical_device, command_pool, graphic_queue,
            MAX_SPACE * 4, num_semaphores, render_state.textures,
            size_arr(render_state.textures), render_state.g_pipline);

        render_state.g_pipline.idx_buffer.data =
            dyn_arrayP(region, MAX_SPACE * 6, u32);
        generate_indices(&render_state.g_pipline.idx_buffer.data, 0, MAX_SPACE);
        render_state.g_pipline.idx_buffer.size_bytes =
            capacity_arr(render_state.g_pipline.idx_buffer.data) * sizeof(u32);
        create_index_buffer(device, physical_device, command_pool, graphic_queue,
                            &render_state.g_pipline.idx_buffer);

        region_pop(region, capacity_arr(render_state.g_pipline.idx_buffer.data), u32,
                   PERM_ARRAY);
        render_state.g_pipline.idx_buffer.data = NULL;

        render_state.font =
            load_font_file(region, "Syntics/res/ArialWhiteSmall.fnt");
        render_state.font.tex_index = 1.0f;

        render_state.mvp.model = mat4i(1.0f);
        render_state.mvp.view = mat4i(1.0f);
    }

#ifdef GAME_ON
    init_platform_game(region, device, physical_device, command_pool, graphic_queue,
                       swap_chain, NUM_SEMAPHORES);
#else
    init_terrain(region, device, physical_device, command_pool, graphic_queue,
                 swap_chain, NUM_SEMAPHORES);

#endif

#ifdef GUI_ON
    gui_init(region, device, physical_device, command_pool, graphic_queue,
             swap_chain, NUM_SEMAPHORES);
#endif

    subscribe(&render_state.key_evt, EVT_KEY);
    subscribe(&render_state.resize_evt, EVT_RESIZE);
}

void create_fence_semaphore(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores)
{
    INIT_0(VkFenceCreateInfo, fence_info);
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    INIT_0(VkSemaphoreCreateInfo, semaphore_info)
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_ASSERT(vkCreateFence(device, &fence_info, NULL, fence));
    VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, NULL, image_semaphores));
    VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, NULL, present_semaphores));
}

void render(Region_Alloc* region, Application_State& app_state, f32 dt)
{
    f32 swap_chain_width = app_state.swap_chain.extent_2D.width;
    f32 swap_chain_height = app_state.swap_chain.extent_2D.height;
    static f32 swap_chain_width_ = swap_chain_width;
    static f32 swap_chain_height_ = swap_chain_height;

    vkWaitForFences(device_handle, 1, &render_state.fences[SEMAPHORE_INDEX], VK_TRUE,
                    UINT64_MAX);

    u32 image_index = 0;
    VkResult result = vkAcquireNextImageKHR(
        device_handle, app_state.swap_chain.swap_chain, UINT64_MAX,
        render_state.image_semaphores[SEMAPHORE_INDEX], VK_NULL_HANDLE,
        &image_index);

    vkResetFences(device_handle, 1, &render_state.fences[SEMAPHORE_INDEX]);

    render_state.mvp.proj =
        ortho(0, 0, swap_chain_width, swap_chain_height, -1.0f, 1.0f);
    update_uniform_buffers(app_state.device,
                           render_state.g_pipline.uniform_buffers[SEMAPHORE_INDEX],
                           &render_state.mvp, sizeof(render_state.mvp));

#ifdef GAME_ON
    update_platform_game(region, device_handle,
                         Vec2(swap_chain_width, swap_chain_height), SEMAPHORE_INDEX,
                         dt);
#else
    update_terrain(region, device_handle, Vec2(swap_chain_width, swap_chain_height),
                   SEMAPHORE_INDEX, dt);
#endif

    begin_render_pass(render_state.command_buffers[SEMAPHORE_INDEX],
                      app_state.swap_chain.render_pass,
                      app_state.swap_chain.framebuffers[image_index],
                      app_state.swap_chain.extent_2D);
    {
        bind_and_draw_graphics_pipline(
            render_state.command_buffers[SEMAPHORE_INDEX],
            render_state.g_pipline.descriptors.desc_sets[SEMAPHORE_INDEX], 0, 100,
            render_state.g_pipline);
#ifdef GAME_ON
        render_platform_game(render_state.command_buffers[SEMAPHORE_INDEX],
                             SEMAPHORE_INDEX);
#else
        render_terrain(render_state.command_buffers[SEMAPHORE_INDEX],
                       SEMAPHORE_INDEX);
#endif

#ifdef GUI_ON
        gui_render(render_state.command_buffers[SEMAPHORE_INDEX], SEMAPHORE_INDEX);
#endif
    }
    end_render_pass(render_state.command_buffers[SEMAPHORE_INDEX]);

    submit_and_present(render_state.queues.graphic_queue,
                       render_state.queues.present_queue,
                       render_state.image_semaphores[SEMAPHORE_INDEX],
                       render_state.present_semaphores[SEMAPHORE_INDEX],
                       render_state.fences[SEMAPHORE_INDEX],
                       render_state.command_buffers[SEMAPHORE_INDEX],
                       app_state.swap_chain.swap_chain, image_index);

    if (is_key_pressed(SYNT_H_PRESSED) && !gui_focus())
    {
#ifdef GAME_ON
        recreate_platform_game(region, app_state);
#else
        recreate_terrain(region, app_state);
#endif
    }

    if (render_state.resize_evt->resize_evt.is_resized ||
        result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        Resize_Evt* e = &render_state.resize_evt->resize_evt;
        e->is_resized = false;
        recreate_swapchain(region, &app_state, e->width, e->height,
                           /*size_arr(render_state.textures)*/ 0);

        recreate_graphic_pipline(region, device_handle, app_state.swap_chain,
                                 "Syntics/res/gui.vert.spv",
                                 "Syntics/res/gui.frag.spv", render_state.g_pipline,
                                 size_arr(render_state.textures), NULL);
#ifdef GAME_ON
        recreate_platform_game(region, app_state);
#else
        recreate_terrain(region, app_state);
#endif
#ifdef GUI_ON
        gui_recreate(region);
#endif
    }

    ++SEMAPHORE_INDEX %= NUM_SEMAPHORES;
}

void submit_and_present(VkQueue graphic_queue, VkQueue present_queue,
                        VkSemaphore image_semaphore, VkSemaphore present_semaphore,
                        VkFence fence, VkCommandBuffer command_buffer,
                        VkSwapchainKHR swap_chain, u32 image_index)
{

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_semaphore;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &present_semaphore;

    VK_ASSERT(vkQueueSubmit(graphic_queue, 1, &submit_info, fence));

    VkPresentInfoKHR present_info = {};
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

    destroy_graphic_pipeline(device_handle, NUM_SEMAPHORES, render_state.g_pipline);

    for (u32 i = 0; i < size_arr(render_state.textures); i++)
    {
        destroy_texture(device_handle, render_state.textures[i]);
    }

#ifdef GUI_ON
    destroy_gui(device_handle, NUM_SEMAPHORES);
#endif

#ifdef GAME_ON
    destroy_platform_game(device_handle, NUM_SEMAPHORES);
#else
    destroy_terrain(device_handle, NUM_SEMAPHORES);
#endif
}

