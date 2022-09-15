#include "buffers.h"
#include "logging.h"
#include <string.h>

namespace synt {

#define RGB(x) x / 255.0f

static void create_alloc_bind(VkDevice device, VkPhysicalDevice physical_device,
                              VkMemoryPropertyFlags wanted_mem_props,
                              VkBufferUsageFlags usage_flags, VkBuffer* buffer,
                              VkDeviceMemory* buffer_memory, VkDeviceSize data_size)
{
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size               = data_size;
    buffer_info.usage              = usage_flags;
    buffer_info.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

    VK_ASSERT(vkCreateBuffer(device, &buffer_info, NULL, buffer));

    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(device, *buffer, &mem_req);

    // Source - Vulkan specification
    // The number of memory types available across all memory heaps is given by
    // memoryTypeCount.

    uint32 mem_type_idx = 0;
    bool found_props    = false;
    for (uint32 i = 0; i < mem_props.memoryTypeCount; i++)
    {
        if (((1 << i) & mem_req.memoryTypeBits) &&
            ((mem_props.memoryTypes[i].propertyFlags & wanted_mem_props) ==
             wanted_mem_props))
        {
            mem_type_idx = i;
            found_props  = true;
            break;
        }
    }

    assert(found_props);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize       = mem_req.size;
    alloc_info.memoryTypeIndex      = mem_type_idx;

    VK_ASSERT(vkAllocateMemory(device, &alloc_info, NULL, buffer_memory));
    VK_ASSERT(vkBindBufferMemory(device, *buffer, *buffer_memory, 0));
}

static void helper_buffer(VkDevice device, VkPhysicalDevice physical_device,
                          void* data, VkDeviceSize data_size_bytes,
                          VkBufferUsageFlags usage_flags,
                          VkDeviceMemory* buffer_memory, VkBuffer* buffer)
{
    // TODO: Can also use a staging buffer. If that is the case: vertex_buffer needs
    // to also have VK_BUFFER_USAGE_TRANSFER_DST_BIT. staging buffer has
    // VK_BUFFER_USAGE_TRANSFER_SRC_BIT. This way vertex_buffer can be a
    // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT. Local to the device.

    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      usage_flags, buffer, buffer_memory, data_size_bytes);

    void* transfer_data = NULL;
    VK_ASSERT(
        vkMapMemory(device, *buffer_memory, 0, data_size_bytes, 0, &transfer_data));
    memcpy(transfer_data, data, (size_t)data_size_bytes);
    vkUnmapMemory(device, *buffer_memory);
}

void create_vertex_buffer(VkDevice device, VkPhysicalDevice physical_device,
                          Vertex_Buffer* vertex_buffer)
{
    assert(vertex_buffer->size_bytes);
    helper_buffer(device, physical_device, vertex_buffer->data,
                  vertex_buffer->size_bytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  &vertex_buffer->buffer_memory, &vertex_buffer->buffer);
}

void create_index_buffer(VkDevice device, VkPhysicalDevice physical_device,
                         Index_Buffer* index_buffer)
{
    assert(index_buffer->size_bytes);
    helper_buffer(device, physical_device, index_buffer->data,
                  index_buffer->size_bytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                  &index_buffer->buffer_memory, &index_buffer->buffer);
}

void create_command_pool(VkDevice device, uint32 queue_fam_index,
                         VkCommandPool* command_pool)
{
    VkCommandPoolCreateInfo create_info = {};
    create_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex        = queue_fam_index;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

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
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_ASSERT(vkAllocateCommandBuffers(device, &alloc_info, command_buffer));
}

void record_execute_commandbuffer(VkCommandBuffer command_buffer,
                                  VkFramebuffer framebuffer, VkExtent2D extent_2D,
                                  VkBuffer vertex_buffer, VkBuffer index_buffer,
                                  uint32 index_count,
                                  const Graphic_Pipline& graphic_pipline)
{
    vkResetCommandBuffer(command_buffer, 0);

    VkCommandBufferBeginInfo buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    VK_ASSERT(vkBeginCommandBuffer(command_buffer, &buffer_begin_info));

    VkClearValue clear_values     = {};
    clear_values.color.float32[0] = RGB(169.0f);
    clear_values.color.float32[1] = RGB(102.0f);
    clear_values.color.float32[2] = RGB(20.0f);
    clear_values.color.float32[3] = 1.0f;

    VkRenderPassBeginInfo render_pass_begin_info = {};
    render_pass_begin_info.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass  = graphic_pipline.render_pass;
    render_pass_begin_info.framebuffer = framebuffer;
    render_pass_begin_info.renderArea.extent = extent_2D;
    render_pass_begin_info.renderArea.offset = (VkOffset2D){ 0, 0 };
    render_pass_begin_info.clearValueCount   = 1;
    render_pass_begin_info.pClearValues      = &clear_values;

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphic_pipline.pipeline);

    VkDeviceSize offset[] = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, offset);
    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    VK_ASSERT(vkEndCommandBuffer(command_buffer));
}

void destroy_buffer(VkDevice device, VkBuffer buffer, VkDeviceMemory buffer_memory)
{
    vkFreeMemory(device, buffer_memory, NULL);
    vkDestroyBuffer(device, buffer, NULL);
}

} // namespace synt
