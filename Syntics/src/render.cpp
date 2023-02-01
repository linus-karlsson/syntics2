#include "render.h"
#include "region_alloc.h"
#include "buffers.h"
#include "event_system.h"
#include "swap_chain.h"
#include "file_reading.h"
#include "gui.h"
#include "terrain.h"
// #include "render-testing.h"
// #include "jailbreak.h"
// #include <stb/stb_truetype.h>
// #include <msdfgen/msdfgen.h>
// #include <msdfgen/msdfgen-ext.h>
#include <string.h>
#include <math.h>
#include <Windows.h>

#define GUI_ON

typedef struct Render_state
{

    VkFence* fences;
    VkSemaphore* image_semaphores;
    VkSemaphore* present_semaphores;

    VkCommandBuffer* command_buffers;

    Queues queues;

    Events* key_evt;
    Events* resize_evt;

} Render_state;

static uint32 NUM_SEMAPHORES = 1;
static uint32 SEMAPHORE_INDEX = 0;
static Render_state render_state = {};
static VkDevice device_handle = VK_NULL_HANDLE;

static int32 maxi32(int32 f, int32 s)
{
    return (f > s) ? f : s;
}

static inline Vec2 mouse_pos_to_pos(const Vec2& mouse_pos, const Vec2& window_size)
{
    // Pos from top left corner (0, 0)
    static const float x_start = -1.0f;
    static const float y_start = -1.0f;

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

for (uint32 i = 0; i < 4; i++)
{
    synt_push(render_state.graphic_piplines[UI_PIPELINE].vert_buffer.data,
              verts[i]);
}

#endif

void init_render_state(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device, VkCommandPool command_pool,
                       const Queue_Family_Indices& q_indices, uint32 num_semaphores,
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

    for (uint32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        create_fence_semaphore(device, &render_state.fences[i],
                               &render_state.image_semaphores[i],
                               &render_state.present_semaphores[i]);

        allocate_commandbuffer(device, command_pool,
                               &render_state.command_buffers[i]);
    }

    init_terrain(region, device, physical_device, command_pool,
                 render_state.queues.graphic_queue, swap_chain, NUM_SEMAPHORES);
#ifdef GUI_ON
    gui_init(region, device, physical_device, command_pool,
             render_state.queues.graphic_queue, swap_chain, NUM_SEMAPHORES);
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

void render(Region_Alloc* region, Application_State& app_state, float dt)
{
    float swap_chain_width = app_state.swap_chain.extent_2D.width;
    float swap_chain_height = app_state.swap_chain.extent_2D.height;
    static float swap_chain_width_ = swap_chain_width;
    static float swap_chain_height_ = swap_chain_height;

    static float test = 0.0f;

    vkWaitForFences(device_handle, 1, &render_state.fences[SEMAPHORE_INDEX], VK_TRUE,
                    UINT64_MAX);

    uint32 image_index = 0;
    VkResult result = vkAcquireNextImageKHR(
        device_handle, app_state.swap_chain.swap_chain, UINT64_MAX,
        render_state.image_semaphores[SEMAPHORE_INDEX], VK_NULL_HANDLE,
        &image_index);

    vkResetFences(device_handle, 1, &render_state.fences[SEMAPHORE_INDEX]);

    update_terrain(region, device_handle, Vec2(swap_chain_width, swap_chain_height),
                   SEMAPHORE_INDEX, dt);

    begin_render_pass(render_state.command_buffers[SEMAPHORE_INDEX],
                      app_state.swap_chain.render_pass,
                      app_state.swap_chain.framebuffers[image_index],
                      app_state.swap_chain.extent_2D);
    {
        render_terrain(render_state.command_buffers[SEMAPHORE_INDEX],
                       SEMAPHORE_INDEX);

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
        recreate_terrain(region, app_state);
    }

    if (render_state.resize_evt->resize_evt.is_resized ||
        result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        Resize_Evt* e = &render_state.resize_evt->resize_evt;
        e->is_resized = false;
        recreate_swapchain(region, &app_state, e->width, e->height,
                           /*size_arr(render_state.textures)*/ 0);
        recreate_terrain(region, app_state);
#ifdef GUI_ON
        gui_recreate(region);
#endif
    }

    ++SEMAPHORE_INDEX %= NUM_SEMAPHORES;
}

void submit_and_present(VkQueue graphic_queue, VkQueue present_queue,
                        VkSemaphore image_semaphore, VkSemaphore present_semaphore,
                        VkFence fence, VkCommandBuffer command_buffer,
                        VkSwapchainKHR swap_chain, uint32 image_index)
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
    for (uint32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        vkDestroyFence(device_handle, render_state.fences[i], NULL);
        vkDestroySemaphore(device_handle, render_state.image_semaphores[i], NULL);
        vkDestroySemaphore(device_handle, render_state.present_semaphores[i], NULL);
    }

#ifdef GUI_ON
    destroy_gui(device_handle, NUM_SEMAPHORES);
#endif

    destroy_terrain(device_handle, NUM_SEMAPHORES);
}

