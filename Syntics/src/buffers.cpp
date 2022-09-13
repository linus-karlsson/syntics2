#include "buffers.h"
namespace synt {

void create_vertex_buffer(VkDevice device,
                          const Queue_Family_Indices& queue_fam_index,
                          VkBuffer* vertex_buffer)
{
    VkBufferCreateInfo buffer_info    = {};
    buffer_info.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.queueFamilyIndexCount = 1;
    buffer_info.pQueueFamilyIndices   = &queue_fam_index.indices[0];
}

void create_index_buffer(VkDevice device,
                         const Queue_Family_Indices& queue_fam_index,
                         VkBuffer* index_buffer)
{
    VkBufferCreateInfo buffer_info    = {};
    buffer_info.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.queueFamilyIndexCount = 1;
    buffer_info.pQueueFamilyIndices   = &queue_fam_index.indices[0];
}

void create_command_pool(VkDevice device, uint32 queue_fam_index,
                         VkCommandPool* command_pool)
{
    VkCommandPoolCreateInfo create_info = {};
    create_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex        = queue_fam_index;

    *command_pool = VK_NULL_HANDLE;
    VK_ASSERT(vkCreateCommandPool(device, &create_info, NULL, command_pool));
}

void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                            VkCommandBuffer* command_buffer)
{

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = command_pool;
    alloc_info.commandBufferCount = 1;

    VK_ASSERT(vkAllocateCommandBuffers(device, &alloc_info, command_buffer));
}

void record_commandbuffer(VkCommandBuffer command_buffer)
{

    VkCommandBufferBeginInfo buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    VK_ASSERT(vkBeginCommandBuffer(command_buffer, &buffer_begin_info));

    VK_ASSERT(vkEndCommandBuffer(command_buffer));
}

void create_fence_semaphore(VkDevice device, VkFence* fence, VkSemaphore* semaphore)
{
    VkFenceCreateInfo fence_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    VkSemaphoreCreateInfo semaphore_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VK_ASSERT(vkCreateFence(device, &fence_info, NULL, fence));
    VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, NULL, semaphore));
}
} // namespace synt
