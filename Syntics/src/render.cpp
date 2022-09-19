#include "render.h"
#include "region_alloc.h"
#include "buffers.h"
#include "camera.h"
#include "event_system.h"
#include "swap_chain.h"
#include <string.h>

namespace synt {

typedef struct Render_state
{
    VkFence* fences;
    VkSemaphore* image_semaphores;
    VkSemaphore* present_semaphores;

    VkCommandBuffer* command_buffers;
    Uniform_Buffer* uniform_buffers;
    Descriptors descriptors;

    Queues queues;

    Camera cam;
    Events* mouse_evt;

} Render_state;

static uint32 NUM_SEMAPHORES           = 2;
static uint32 SEMAPHORE_INDEX          = 0;
static Render_state render_state       = {};
static VkDevice internal_device_handle = VK_NULL_HANDLE;

void init_render_state(Region_Alloc* region, VkDevice device, Queues queues,
                       VkPhysicalDevice physical_device, VkCommandPool command_pool,
                       VkDescriptorSetLayout desc_layout, const Texture& texture,
                       const Queue_Family_Indices& q_indices, uint32 num_semaphores)
{
    NUM_SEMAPHORES = num_semaphores;

    render_state.queues = queues;

    internal_device_handle = device;

    render_state.fences = region_mallocP((*region), NUM_SEMAPHORES, VkFence);

    render_state.image_semaphores =
        region_mallocP((*region), NUM_SEMAPHORES, VkSemaphore);

    render_state.present_semaphores =
        region_mallocP((*region), NUM_SEMAPHORES, VkSemaphore);

    render_state.command_buffers =
        region_mallocP((*region), NUM_SEMAPHORES, VkCommandBuffer);

    render_state.uniform_buffers =
        region_mallocP((*region), NUM_SEMAPHORES, Uniform_Buffer);

    render_state.descriptors.desc_sets =
        region_mallocP((*region), NUM_SEMAPHORES, VkDescriptorSet);

    for (uint32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        create_fence_semaphore(device, &render_state.fences[i],
                               &render_state.image_semaphores[i],
                               &render_state.present_semaphores[i]);

        allocate_commandbuffer(device, command_pool, &render_state.command_buffers[i]);

        render_state.uniform_buffers[i].size_bytes = (uint32)sizeof(MVP);
        create_uniform_buffer(device, physical_device, &render_state.uniform_buffers[i]);
    }

    create_descriptors(device, &render_state.descriptors, NUM_SEMAPHORES, desc_layout,
                       texture, render_state.uniform_buffers);

    render_state.cam.mvp.model =
        scale(rotate(mat4i(1.0f), (float)radians(1.0f), X), v3f(1.0f, 1.0f, 1.0f));
    render_state.cam.speed = 2.0f;

    render_state.cam.position    = synt::v3f(0.0f, 0.0f, 1.0f);
    render_state.cam.orientation = synt::v3f(0.0f, 0.0f, -1.0f);

    subscribe(&render_state.mouse_evt, EVT_MOUSE);
}

void create_fence_semaphore(VkDevice device, VkFence* fence,
                            VkSemaphore* image_semaphores,
                            VkSemaphore* present_semaphores)
{
    VkFenceCreateInfo fence_info = {};
    fence_info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphore_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VK_ASSERT(vkCreateFence(device, &fence_info, NULL, fence));
    VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, NULL, image_semaphores));
    VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, NULL, present_semaphores));
}

void render(Region_Alloc* region, Application_State& app_state, float dt)
{
    float swap_chain_width  = app_state.swap_chain.extent_2D.width;
    float swap_chain_height = app_state.swap_chain.extent_2D.height;

    static float test = 0.0f;

    vkWaitForFences(internal_device_handle, 1, &render_state.fences[SEMAPHORE_INDEX],
                    VK_TRUE, UINT64_MAX);

    uint32 image_index = 0;
    VkResult result    = vkAcquireNextImageKHR(
           internal_device_handle, app_state.swap_chain.swap_chain, UINT64_MAX,
           render_state.image_semaphores[SEMAPHORE_INDEX], VK_NULL_HANDLE, &image_index);

    vkResetFences(internal_device_handle, 1, &render_state.fences[SEMAPHORE_INDEX]);

    update_camera(&render_state.cam, render_state.mouse_evt, dt);

    if (is_key_pressed(SYNT_E_PRESSED)) test += 60.0f * dt;
    if (is_key_pressed(SYNT_Q_PRESSED)) test -= 60.0f * dt;

    render_state.cam.mvp.model =
        scale(rotate(mat4i(1.0f), (float)radians(test), X), v3f(1.0f, 1.0f, 1.0f));

    render_state.cam.mvp.proj =
        perspective(radians(53.0f), swap_chain_width / swap_chain_height, 0.1f, 100.0f);

    static uint32 one = 0;
    if (one < NUM_SEMAPHORES)
    {
        void* transer_data;
        vkMapMemory(internal_device_handle,
                    render_state.uniform_buffers[SEMAPHORE_INDEX].buffer_memory, 0,
                    sizeof(MVP), 0, &transer_data);
        memcpy(transer_data, &render_state.cam.mvp, sizeof(render_state.cam.mvp));
        vkUnmapMemory(internal_device_handle,
                      render_state.uniform_buffers[SEMAPHORE_INDEX].buffer_memory);
        one++;
    }

    record_execute_commandbuffer(
        render_state.command_buffers[SEMAPHORE_INDEX],
        app_state.swap_chain.framebuffers[image_index], app_state.swap_chain.extent_2D,
        app_state.vert_buffer.buffer, app_state.idx_buffer.buffer,
        app_state.idx_buffer.size_bytes / sizeof(uint32),
        render_state.descriptors.desc_sets[SEMAPHORE_INDEX],
        app_state.swap_chain.graphic_pipline);

    submit_and_present(render_state.queues.graphic_queue,
                       render_state.queues.present_queue,
                       render_state.image_semaphores[SEMAPHORE_INDEX],
                       render_state.present_semaphores[SEMAPHORE_INDEX],
                       render_state.fences[SEMAPHORE_INDEX],
                       render_state.command_buffers[SEMAPHORE_INDEX],
                       app_state.swap_chain.swap_chain, image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        uint16 width, height;
        get_window_size(&width, &height);
        recreate_swapchain(region, &app_state, width, height);
    }

    static float sec = 0;
    sec += dt;
    if (sec >= 0.5)
    {
        // synt_LOG("(x: %f, y: %f, z:%f)\n", render_state.cam.position.x,
        //          render_state.cam.position.y, render_state.cam.position.z);
        sec = 0;
    }
    ray_casting_ex(app_state.device, app_state.phy_device, render_state.cam.position,
                   render_state.cam.orientation, app_state.com_pool,
                   render_state.queues.graphic_queue, &app_state.texture);

    if (++SEMAPHORE_INDEX >= NUM_SEMAPHORES) SEMAPHORE_INDEX = 0;
}

void submit_and_present(VkQueue graphic_queue, VkQueue present_queue,
                        VkSemaphore image_semaphore, VkSemaphore present_semaphore,
                        VkFence fence, VkCommandBuffer command_buffer,
                        VkSwapchainKHR swap_chain, uint32 image_index)
{

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info         = {};
    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = &image_semaphore;
    submit_info.pWaitDstStageMask    = &wait_stage;
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = &command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = &present_semaphore;

    VK_ASSERT(vkQueueSubmit(graphic_queue, 1, &submit_info, fence));

    VkPresentInfoKHR present_info   = {};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &present_semaphore;
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &swap_chain;
    present_info.pImageIndices      = &image_index;

    vkQueuePresentKHR(present_queue, &present_info);
}

void destroy_render_state()
{

    for (uint32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        vkDestroyFence(internal_device_handle, render_state.fences[i], NULL);
        vkDestroySemaphore(internal_device_handle, render_state.image_semaphores[i],
                           NULL);
        vkDestroySemaphore(internal_device_handle, render_state.present_semaphores[i],
                           NULL);

        destroy_buffer(internal_device_handle, render_state.uniform_buffers[i].buffer,
                       render_state.uniform_buffers[i].buffer_memory);
    }

    vkDestroyDescriptorPool(internal_device_handle, render_state.descriptors.desc_pool,
                            NULL);
}
} // namespace synt
