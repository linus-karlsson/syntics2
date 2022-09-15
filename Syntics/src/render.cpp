#include "render.h"
#include "region_alloc.h"
#include "buffers.h"

namespace synt {

typedef struct Render_state
{
    VkFence* fences;
    VkSemaphore* image_semaphores;
    VkSemaphore* present_semaphores;

    VkCommandBuffer* command_buffers;

    VkQueue graphic_queue;
    VkQueue present_queue;

} Render_state;

static uint32 NUM_SEMAPHORES              = 2;
static uint32 SEMAPHORE_INDEX             = 0;
static Render_state internal_render_state = {};
static VkDevice internal_device_handle    = VK_NULL_HANDLE;

void init_render_state(Region_Alloc* region, VkDevice device, VkCommandPool command_pool,
                       const Queue_Family_Indices& q_indices, uint32 num_semaphores)
{
    NUM_SEMAPHORES = num_semaphores;

    internal_device_handle = device;

    vkGetDeviceQueue(device, q_indices.indices[GRAPHICS_QUEUE_IDX], 0,
                     &internal_render_state.graphic_queue);

    vkGetDeviceQueue(device, q_indices.indices[GRAPHICS_QUEUE_IDX], 0,
                     &internal_render_state.present_queue);

    internal_render_state.fences = region_mallocP((*region), NUM_SEMAPHORES, VkFence);

    internal_render_state.image_semaphores =
        region_mallocP((*region), NUM_SEMAPHORES, VkSemaphore);

    internal_render_state.present_semaphores =
        region_mallocP((*region), NUM_SEMAPHORES, VkSemaphore);

    internal_render_state.command_buffers =
        region_mallocP((*region), NUM_SEMAPHORES, VkCommandBuffer);

    for (uint32 i = 0; i < NUM_SEMAPHORES; i++)
    {
        create_fence_semaphore(device, &internal_render_state.fences[i],
                               &internal_render_state.image_semaphores[i],
                               &internal_render_state.present_semaphores[i]);

        allocate_commandbuffer(device, command_pool,
                               &internal_render_state.command_buffers[i]);
    }
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

void render(Region_Alloc* region, const Swap_Chain_attrib& swap_chain,
            const Vertex_Buffer& vertex_buffer, const Index_Buffer& index_buffer)
{

    vkWaitForFences(internal_device_handle, 1,
                    &internal_render_state.fences[SEMAPHORE_INDEX], VK_TRUE, UINT64_MAX);

    uint32 image_index = 0;
    VK_ASSERT(
        vkAcquireNextImageKHR(internal_device_handle, swap_chain.swap_chain, UINT64_MAX,
                              internal_render_state.image_semaphores[SEMAPHORE_INDEX],
                              VK_NULL_HANDLE, &image_index));

    vkResetFences(internal_device_handle, 1,
                  &internal_render_state.fences[SEMAPHORE_INDEX]);

    record_execute_commandbuffer(
        internal_render_state.command_buffers[SEMAPHORE_INDEX],
        swap_chain.framebuffers[image_index], swap_chain.extent_2D, vertex_buffer.buffer,
        index_buffer.buffer, size_arr(index_buffer.data), swap_chain.graphic_pipline);

    submit_and_present(internal_render_state.graphic_queue,
                       internal_render_state.present_queue,
                       internal_render_state.image_semaphores[SEMAPHORE_INDEX],
                       internal_render_state.present_semaphores[SEMAPHORE_INDEX],
                       internal_render_state.fences[SEMAPHORE_INDEX],
                       internal_render_state.command_buffers[SEMAPHORE_INDEX],
                       swap_chain.swap_chain, image_index);

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
        vkDestroyFence(internal_device_handle, internal_render_state.fences[i], NULL);
        vkDestroySemaphore(internal_device_handle,
                           internal_render_state.image_semaphores[i], NULL);
        vkDestroySemaphore(internal_device_handle,
                           internal_render_state.present_semaphores[i], NULL);
    }
}
} // namespace synt
