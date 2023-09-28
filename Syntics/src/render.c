#ifndef SY_UNIT_BUILD
#include "render.h"
#include "logging.h"
#include "region_alloc.h"
#include "buffers.h"
#include "event_system.h"
#include "swap_chain.h"
#include "font.h"
#include "collision.h"
#include "vulkan_types.h"
#endif

typedef struct Update_Task
{
    void (*update_callback_p)(void* data, Region_Alloc* region,
                              const Application_State* app_state,
                              Render_State* render_state, V2 dimensions,
                              u32 semaphore_idx, f32 dt);
    void* data;
} Update_Task;

typedef struct Recreate_Task
{
    void (*rc_callback)(void* data, const Application_State* app_state);
    void* data;
} Recreate_Task;

typedef struct Recreate_Graphic_Pipeline_Task
{
    void (*rc_gp_callback)(void* data, const Application_State* app_state);
    void* data;
} Recreate_Graphic_Pipeline_Task;

typedef struct Destroy_Task
{
    void (*destroy_callback)(void* data, VkDevice device, u32 num_semaphores);
    void* data;
} Destroy_Task;

typedef struct Render_State_Internal
{
    VkFence* fences;
    VkSemaphore* image_semaphores;
    VkSemaphore* present_semaphores;

    VkCommandBuffer* command_buffers;

    Queues queues;

    // Topbar and other utilities
    VkPipeline g_pipeline;
    VP vp;
    Font font;
    Rect2D* rects;

    Events* key_evt;
    Events* resize_evt;

    Texture* textures;
    Update_Task* update_tasks;
    Recreate_Task* rc_tasks;
    Recreate_Graphic_Pipeline_Task* rc_gp_tasks;
    Destroy_Task* destroy_tasks;

    u32 semaphore_index;
    u32 image_index;

    b8 file_changed;
    char* path_to_detect;
    File_Change_Handle file_change_handle;
    Semaphore start_semaphore;

} Render_State_Internal;

#if 0
thread_return_value looking_for_file_changes(void* data)
{
    Render_State_Internal* state = (Render_State_Internal*)data;
    for (;;)
    {
        semaphore_wait_and_decrement(&state->start_semaphore);
        state->file_change_handle = FindFirstChangeNotification(
            state->path_to_detect, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

        assert(state->file_change_handle != INVALID_HANDLE_VALUE);

        WaitForSingleObject(state->file_change_handle, INFINITE);

        state->file_changed = true;
    }
}
#endif

global u32 NUM_SEMAPHORES = 0;
#define RENDER_MAX_SPACE 100

u32 semaphore_idx_get(Render_State* render_state)
{
    Render_State_Internal* state_internal =
        (Render_State_Internal*)render_state;
    return state_internal->semaphore_index;
}

void fence_semaphore_create(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores)
{
    VkFenceCreateInfo fence_info = { 0 };
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphore_info = { 0 };
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_ASSERT(vkCreateFence(device, &fence_info, NULL, fence));
    VK_ASSERT(
        vkCreateSemaphore(device, &semaphore_info, NULL, image_semaphores));
    VK_ASSERT(
        vkCreateSemaphore(device, &semaphore_info, NULL, present_semaphores));
}

void render_state_init(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device,
                       VkCommandPool command_pool,
                       const Queue_Family_Indices* q_indices,
                       u32 num_semaphores, const Swap_Chain_Attrib* swap_chain,
                       const Platform* platform, Render_State** render_state)
{
    Render_State_Internal* state_internal =
        region_calloc(region, 1, Render_State_Internal);

    state_internal->start_semaphore = semaphore_create(0, 1);

#if 0 
    const char* p = "Syntics/res/shaders/spv";
    state_internal->path_to_detect = path_extend(region, p, (u32)strlen(p));

    thread_create(state_internal, looking_for_file_changes, 0, NULL);
    ReleaseSemaphore(state_internal->start_semaphore, 1, 0);
#endif

    state_internal->queues = queues;

    NUM_SEMAPHORES = num_semaphores;

    state_internal->fences = region_malloc(region, NUM_SEMAPHORES, VkFence);
    state_internal->image_semaphores =
        region_malloc(region, NUM_SEMAPHORES, VkSemaphore);
    state_internal->present_semaphores =
        region_malloc(region, NUM_SEMAPHORES, VkSemaphore);
    state_internal->command_buffers =
        region_malloc(region, NUM_SEMAPHORES, VkCommandBuffer);

    for (u32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        fence_semaphore_create(device, &state_internal->fences[i],
                               &state_internal->image_semaphores[i],
                               &state_internal->present_semaphores[i]);
    }
    commandbuffers_allocate(device, command_pool,
                            VK_COMMAND_BUFFER_LEVEL_PRIMARY, NUM_SEMAPHORES,
                            state_internal->command_buffers);

    state_internal->update_tasks = region_array(region, 10, Update_Task);
    state_internal->rc_tasks = region_array(region, 10, Recreate_Task);
    state_internal->rc_gp_tasks =
        region_array(region, 10, Recreate_Graphic_Pipeline_Task);
    state_internal->destroy_tasks = region_array(region, 10, Destroy_Task);

    VkQueue graphic_queue = state_internal->queues.graphic_queue;

    event_subscribe(&state_internal->key_evt, EVT_KEY);
    event_subscribe(&state_internal->resize_evt, EVT_RESIZE);

    *render_state = (Render_State*)state_internal;
}

VkQueue graphic_queue_get(Render_State* render_state)
{
    Render_State_Internal* state_internal =
        (Render_State_Internal*)render_state;
    return state_internal->queues.graphic_queue;
}

void subscribe_update_callback(
    Render_State* render_state,
    void (*update_callback_p)(void* data, Region_Alloc* region,
                              const Application_State* app_state,
                              Render_State* render_state, V2 dimensions,
                              u32 semaphore_idx, f32 dt),
    void* data)
{
    Render_State_Internal* state_internal =
        (Render_State_Internal*)render_state;

    Update_Task task = { update_callback_p, data };
    array_push(state_internal->update_tasks, task);
}

void subscribe_recreate_callback(
    Render_State* render_state,
    void (*rc_callback)(void* data, const Application_State* app_state),
    void* data)
{
    Render_State_Internal* state_internal =
        (Render_State_Internal*)render_state;

    Recreate_Task task = { rc_callback, data };
    array_push(state_internal->rc_tasks, task);
}

void subscribe_recreate_gp_callback(
    Render_State* render_state,
    void (*rc_gp_callback)(void* data, const Application_State* app_state),
    void* data)
{
    Render_State_Internal* state_internal =
        (Render_State_Internal*)render_state;

    Recreate_Graphic_Pipeline_Task task = { rc_gp_callback, data };
    array_push(state_internal->rc_gp_tasks, task);
}

void subscribe_destroy_callback(Render_State* render_state,
                                void (*destroy_callback)(void* data,
                                                         VkDevice device,
                                                         u32 num_semaphores),
                                void* data)
{
    Render_State_Internal* state_internal =
        (Render_State_Internal*)render_state;

    Destroy_Task task = { destroy_callback, data };
    array_push(state_internal->destroy_tasks, task);
}

void submit_and_present(VkQueue graphic_queue, VkQueue present_queue,
                        VkSemaphore image_semaphore,
                        VkSemaphore present_semaphore, VkFence fence,
                        VkCommandBuffer* command_buffers,
                        u32 command_buffer_count, VkSwapchainKHR swap_chain,
                        u32 image_index)
{

    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info = { 0 };
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_semaphore;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.pCommandBuffers = command_buffers;
    submit_info.commandBufferCount = command_buffer_count;
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

void frame_begin(Render_State* render_state, Application_State* app_state)
{
    Render_State_Internal* state_internal =
        (Render_State_Internal*)render_state;

    vkWaitForFences(app_state->device, 1,
                    &state_internal->fences[state_internal->semaphore_index],
                    VK_TRUE, UINT64_MAX);

    vkResetFences(app_state->device, 1,
                  &state_internal->fences[state_internal->semaphore_index]);

    state_internal->image_index = 0;
    VkResult result = vkAcquireNextImageKHR(
        app_state->device, app_state->swap_chain.swap_chain, UINT64_MAX,
        state_internal->image_semaphores[state_internal->semaphore_index],
        VK_NULL_HANDLE, &state_internal->image_index);

    if (state_internal->resize_evt->resize_evt.is_resized ||
        result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        Resize_Evt* e = &state_internal->resize_evt->resize_evt;
        e->is_resized = false;
        swapchain_recreate(app_state, e->width, e->height);

        u32 size = array_size(state_internal->rc_tasks);
        for (u32 i = 0; i < size; i++)
        {
            Recreate_Task* t = &state_internal->rc_tasks[i];
            t->rc_callback(t->data, app_state);
        }
    }
}

void frame_render(Render_State* render_state, Application_State* app_state,
                  Render_Task* copy_tasks, Render_Task* render_tasks, f32 dt)
{
    Render_State_Internal* state_internal =
        (Render_State_Internal*)render_state;

    vkResetCommandBuffer(
        state_internal->command_buffers[state_internal->semaphore_index], 0);

    VkCommandBufferBeginInfo buffer_begin_info = { 0 };
    buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_ASSERT(vkBeginCommandBuffer(
        state_internal->command_buffers[state_internal->semaphore_index],
        &buffer_begin_info));
    {
        const u32 size = array_size(copy_tasks);
        for (u32 i = 0; i < size; i++)
        {
            Render_Task* t = copy_tasks + i;
            t->callback(t->data,
                        state_internal
                            ->command_buffers[state_internal->semaphore_index],
                        state_internal->semaphore_index);
        }
    }
    render_pass_begin(
        state_internal->command_buffers[state_internal->semaphore_index],
        app_state->swap_chain.render_pass,
        app_state->swap_chain.framebuffers[state_internal->image_index],
        &app_state->swap_chain.extent_2D);
    {
        const u32 size = array_size(render_tasks);
        for (u32 i = 0; i < size; i++)
        {
            Render_Task* t = render_tasks + i;
            t->callback(t->data,
                        state_internal
                            ->command_buffers[state_internal->semaphore_index],
                        state_internal->semaphore_index);
        }
    }
    render_pass_end(
        state_internal->command_buffers[state_internal->semaphore_index]);

    submit_and_present(
        state_internal->queues.graphic_queue,
        state_internal->queues.present_queue,
        state_internal->image_semaphores[state_internal->semaphore_index],
        state_internal->present_semaphores[state_internal->semaphore_index],
        state_internal->fences[state_internal->semaphore_index],
        &state_internal->command_buffers[state_internal->semaphore_index], 1,
        app_state->swap_chain.swap_chain, state_internal->image_index);

    // NOTE: this should not be here, it should be in the main loop. Stop all
    // threads and frames and recreate. Then start them up again.
#if 0
    if (state_internal->file_changed)
    {
        // TODO: Because more than one file gets compile each time this function gets
        // called multiple times
        u32 size = array_size(state_internal->rc_gp_tasks);
        for (u32 i = 0; i < size; i++)
        {
            Recreate_Graphic_Pipeline_Task* t = &state_internal->rc_gp_tasks[i];
            t->rc_gp_callback(t->data, app_state);
        }
        state_internal->file_changed = false;
        ReleaseSemaphore(state_internal->start_semaphore, 1, 0);
    }
#endif

    state_internal->semaphore_index++;
    state_internal->semaphore_index %= NUM_SEMAPHORES;
}

void render_state_destroy(VkDevice device, Render_State* render_state)
{
    Render_State_Internal* rsi = (Render_State_Internal*)render_state;
    for (u32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        vkDestroyFence(device, rsi->fences[i], NULL);
        vkDestroySemaphore(device, rsi->image_semaphores[i], NULL);
        vkDestroySemaphore(device, rsi->present_semaphores[i], NULL);
    }

#ifdef CUSTOM_TOP_BAR
    graphic_pipeline_destroy(device, NUM_SEMAPHORES, rsi->g_pipeline);

    for (u32 i = 0; i < array_size(rsi->textures); i++)
    {
        texture_destroy(device, rsi->textures[i]);
    }
#endif
}
