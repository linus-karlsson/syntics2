#include "buffers.h"
#include "defines.h"
#include "logging.h"
#include "stb/stb_image.h"
#include "region_alloc.h"
#include "vulkan_types.h"
#include <math.h>
#include <string.h>

// TODO: Need to fix this
void create_image_view(VkDevice device, VkImage image,
                       VkImageViewType image_view_type, VkFormat image_format,
                       VkImageAspectFlags aspect_mask, u32 mip_map_lvl,
                       VkImageView* image_view);

#define RGB(x) x / 255.0f

static i32 get_type_index(VkPhysicalDeviceMemoryProperties mem_props,
                          VkMemoryRequirements mem_req,
                          VkMemoryPropertyFlags wanted_mem_props)
{
    for (u32 i = 0; i < mem_props.memoryTypeCount; i++)
    {
        if (((1 << i) & mem_req.memoryTypeBits) &&
            ((mem_props.memoryTypes[i].propertyFlags & wanted_mem_props) ==
             wanted_mem_props))
        {
            return i;
        }
    }
    return -1;
}

static void create_alloc_bind(VkDevice device, VkPhysicalDevice physical_device,
                              VkMemoryPropertyFlags wanted_mem_props,
                              VkBufferUsageFlags usage_flags, VkBuffer* buffer,
                              VkDeviceMemory* buffer_memory, VkDeviceSize data_size)
{
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = data_size;
    buffer_info.usage = usage_flags;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_ASSERT(vkCreateBuffer(device, &buffer_info, NULL, buffer));

    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(device, *buffer, &mem_req);

    i32 mem_type_idx = get_type_index(mem_props, mem_req, wanted_mem_props);
    ASSERT(mem_type_idx != -1, "");

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = (u32)mem_type_idx;

    VK_ASSERT(vkAllocateMemory(device, &alloc_info, NULL, buffer_memory));
    VK_ASSERT(vkBindBufferMemory(device, *buffer, *buffer_memory, 0));
}

static void helper_buffer(VkDevice device, VkPhysicalDevice physical_device,
                          void* data, VkBufferUsageFlags usage_flags, Buffer* buffer)
{
    create_alloc_bind(
        device, physical_device,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        usage_flags, &buffer->buffer, &buffer->buffer_memory, buffer->size_bytes);

    map_copy_unmap_mem(device, buffer, data);
}

static void staging_buffers(VkDevice device, VkPhysicalDevice physical_device,
                            VkCommandPool command_pool, VkQueue graphics_queue,
                            VkBufferUsageFlags vertex_or_index, void* data,
                            VkBuffer* buffer, VkDeviceMemory* buffer_memory,
                            VkDeviceSize size_bytes)
{
    Buffer staging_buffer = {};
    staging_buffer.size_bytes = size_bytes;
    ASSERT(staging_buffer.size_bytes, "");
    ASSERT(data, "data is null");

    helper_buffer(device, physical_device, data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  &staging_buffer);

    create_alloc_bind(device, physical_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      vertex_or_index | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer,
                      buffer_memory, size_bytes);

    copy_buffer(device, command_pool, staging_buffer.buffer, *buffer, graphics_queue,
                size_bytes);

    destroy_buffer(device, staging_buffer);
}

VkCommandBuffer begin_command_buffer(VkDevice device, VkCommandPool command_pool)
{
    VkCommandBuffer command_buff = VK_NULL_HANDLE;
    allocate_commandbuffer(device, command_pool, &command_buff);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    vkBeginCommandBuffer(command_buff, &begin_info);

    return command_buff;
}

void end_command_buffer(VkDevice device, VkCommandPool command_pool,
                        VkCommandBuffer command_buff, VkQueue graphics_queue)
{
    vkEndCommandBuffer(command_buff);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buff;

    vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buff);
}

void copy_buffer(VkDevice device, VkCommandPool command_pool, VkBuffer src_buffer,
                 VkBuffer dst_buffer, VkQueue graphics_queue,
                 VkDeviceSize size_bytes)
{
    VkCommandBuffer command_buff = begin_command_buffer(device, command_pool);

    VkBufferCopy buff_copy = {};
    buff_copy.size = size_bytes;
    vkCmdCopyBuffer(command_buff, src_buffer, dst_buffer, 1, &buff_copy);

    end_command_buffer(device, command_pool, command_buff, graphics_queue);
}

void map_copy_mem(VkDevice device, Buffer* buffer, void* data)
{
    buffer->transfer_data = NULL;
    if (vkMapMemory(device, buffer->buffer_memory, 0, buffer->size_bytes, 0,
                    &buffer->transfer_data))
    {
        SY_ERROR("vkMapMemory failed\n");
    }
    if (data)
    {
        memcpy(buffer->transfer_data, data, (size_t)buffer->size_bytes);
    }
}

void map_copy_mem_index(VkDevice device, Index_Buffer* ib)
{
    ib->data = NULL;
    if (vkMapMemory(device, ib->buffer.buffer_memory, 0, ib->buffer.size_bytes, 0,
                    (void**)&ib->data))
    {
        SY_ERROR("vkMapMemory failed\n");
    }
}

void map_copy_mem_vertex(VkDevice device, Vertex_Buffer* vb)
{
    vb->data = NULL;
    if (vkMapMemory(device, vb->buffer.buffer_memory, 0, vb->buffer.size_bytes, 0,
                    (void**)&vb->data))
    {
        SY_ERROR("vkMapMemory failed\n");
    }
}

void map_copy_unmap_mem(VkDevice device, Buffer* buffer, void* data)
{
    buffer->transfer_data = NULL;
    if (vkMapMemory(device, buffer->buffer_memory, 0, buffer->size_bytes, 0,
                    &buffer->transfer_data))
    {
        SY_ERROR("vkMapMemory failed\n");
    }
    if (data)
    {
        memcpy(buffer->transfer_data, data, (size_t)buffer->size_bytes);
    }
    vkUnmapMemory(device, buffer->buffer_memory);
}

void create_vertex_buffer_test(VkDevice device, VkPhysicalDevice physical_device,
                               Vertex_Buffer* vertex_buffer)
{
    Buffer* b = &vertex_buffer->buffer;
    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &b->buffer,
                      &b->buffer_memory, b->size_bytes);

    map_copy_mem_vertex(device, vertex_buffer);
}

void create_vertex_buffer_visible(VkDevice device, VkPhysicalDevice physical_device,
                                  Vertex_Buffer* vertex_buffer)
{
    Buffer* b = &vertex_buffer->buffer;
    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &b->buffer,
                      &b->buffer_memory, b->size_bytes);

    map_copy_mem(device, b, vertex_buffer->data);
}

void create_vertex_buffer_local(VkDevice device, VkPhysicalDevice physical_device,
                                VkCommandPool command_pool, VkQueue graphics_queue,
                                Vertex_Buffer* vertex_buffer)
{
    Buffer* b = &vertex_buffer->buffer;
    staging_buffers(device, physical_device, command_pool, graphics_queue,
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_buffer->data,
                    &b->buffer, &b->buffer_memory, b->size_bytes);
}

void create_index_buffer_test(VkDevice device, VkPhysicalDevice physical_device,
                              Index_Buffer* index_buffer)
{
    Buffer* b = &index_buffer->buffer;
    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &b->buffer,
                      &b->buffer_memory, b->size_bytes);

    map_copy_mem_index(device, index_buffer);
}

void create_index_buffer_visible(VkDevice device, VkPhysicalDevice physical_device,
                                 Index_Buffer* index_buffer)
{
    Buffer* b = &index_buffer->buffer;
    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &b->buffer,
                      &b->buffer_memory, b->size_bytes);

    map_copy_mem(device, b, index_buffer->data);
}

void create_index_buffer_local(VkDevice device, VkPhysicalDevice physical_device,
                               VkCommandPool command_pool, VkQueue graphics_queue,
                               Index_Buffer* index_buffer)
{
    Buffer* b = &index_buffer->buffer;
    staging_buffers(device, physical_device, command_pool, graphics_queue,
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_buffer->data, &b->buffer,
                    &b->buffer_memory, b->size_bytes);
}

void create_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device,
                           Uniform_Buffer* uniform_buffer)
{
    Buffer* b = &uniform_buffer->buffer;
    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &b->buffer,
                      &b->buffer_memory, b->size_bytes);

    b->transfer_data = NULL;
    if (vkMapMemory(device, b->buffer_memory, 0, b->size_bytes, 0,
                    &b->transfer_data))
    {
        SY_ERROR("vkMapMemory failed\n");
    }
}

void create_uniform_buffer_test(VkDevice device, VkPhysicalDevice physical_device,
                                Uniform_Buffer* uniform_buffer, void** data)
{
    Buffer* b = &uniform_buffer->buffer;
    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &b->buffer,
                      &b->buffer_memory, b->size_bytes);

    *data = NULL;
    if (vkMapMemory(device, b->buffer_memory, 0, b->size_bytes, 0, data))
    {
        SY_ERROR("vkMapMemory failed\n");
    }
}

void create_command_pool(VkDevice device, u32 queue_fam_index,
                         VkCommandPool* command_pool)
{
    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = queue_fam_index;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    *command_pool = VK_NULL_HANDLE;
    VK_ASSERT(vkCreateCommandPool(device, &create_info, NULL, command_pool));
}

void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                            VkCommandBuffer* command_buffer)
{
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_ASSERT(vkAllocateCommandBuffers(device, &alloc_info, command_buffer));
}

void update_descritors(Region_Alloc* region, VkDevice device,
                       Descriptors* desciptors, u32 desc_count,
                       const Texture* textures, u32 num_textures,
                       Uniform_Buffer* uniform_buffers)
{
    stack_begin_scope();

    for_range(i, desc_count)
    {
#if 1
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = uniform_buffers[i].buffer.buffer;
        buffer_info.range = sizeof(MVP);
#endif

        VkDescriptorImageInfo* image_infos =
            stack_malloc(num_textures, VkDescriptorImageInfo);

        for_range(j, num_textures)
        {
            VkDescriptorImageInfo image_info = {};
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView = textures[j].img_view;
            image_info.sampler = textures[j].texture_sampler;

            image_infos[j] = image_info;
        }

#if 1
        VkWriteDescriptorSet desc_writes[2] = {};
        desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        desc_writes[0].pBufferInfo = &buffer_info;
        desc_writes[0].dstSet = desciptors->desc_sets[i];
        desc_writes[0].dstBinding = 0;

        desc_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_writes[1].descriptorCount = num_textures;
        desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        desc_writes[1].pImageInfo = image_infos;
        desc_writes[1].dstSet = desciptors->desc_sets[i];
        desc_writes[1].dstBinding = 1;
#else
        VkWriteDescriptorSet desc_writes[1] = { 0 };
        desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_writes[0].descriptorCount = num_textures;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        desc_writes[0].pImageInfo = image_infos;
        desc_writes[0].dstSet = desciptors->desc_sets[i];
        desc_writes[0].dstBinding = 0;
#endif

        vkUpdateDescriptorSets(device, sy_SIZE(desc_writes), desc_writes, 0, NULL);
    }

    stack_end_scope();
}

void create_descriptors(Region_Alloc* region, VkDevice device,
                        Descriptors* desciptors, u32 desc_count,
                        VkDescriptorSetLayout desc_layout, const Texture* texture,
                        u32 num_textures, Uniform_Buffer* uniform_buffers)
{
    stack_begin_scope();

    desciptors->desc_count = desc_count;

#if 1
    VkDescriptorPoolSize pool_sizes[2] = {  };
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = desc_count;

    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = desc_count * num_textures;
#else
    VkDescriptorPoolSize pool_sizes[1] = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[0].descriptorCount = desc_count * num_textures;
#endif

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = desc_count;
    pool_info.poolSizeCount = sy_SIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VK_ASSERT(
        vkCreateDescriptorPool(device, &pool_info, NULL, &desciptors->desc_pool));

    if (!desciptors->desc_sets) SY_ERROR("Need to allocate descriptor sets");

    VkDescriptorSetLayout* set_layouts =
        stack_malloc(desc_count, VkDescriptorSetLayout);

    for_range(i, desc_count)
    {
        set_layouts[i] = desc_layout;
    }
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = desciptors->desc_pool;
    alloc_info.descriptorSetCount = desc_count;
    alloc_info.pSetLayouts = set_layouts;

    VK_ASSERT(vkAllocateDescriptorSets(device, &alloc_info, desciptors->desc_sets));

    update_descritors(region, device, desciptors, desc_count, texture, num_textures,
                      uniform_buffers);

    stack_end_scope();
}

void create_image(u32 width, u32 height, VkDevice device,
                  VkPhysicalDevice physical_device, VkFormat format,
                  VkImageTiling tiling, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags wanted_mem_props, VkImage* image,
                  VkDeviceMemory* image_mem, u32 mip_map_lvl,
                  VkSampleCountFlagBits num_samples)
{

    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = mip_map_lvl;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = num_samples;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_ASSERT(vkCreateImage(device, &image_info, NULL, image));

    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(device, *image, &mem_req);

    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    i32 mem_type_idx = get_type_index(mem_props, mem_req, wanted_mem_props);
    ASSERT(mem_type_idx != -1, "");

    VkMemoryAllocateInfo mem_alloc_info = {};
    mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc_info.allocationSize = mem_req.size;
    mem_alloc_info.memoryTypeIndex = (u32)mem_type_idx;

    VK_ASSERT(vkAllocateMemory(device, &mem_alloc_info, NULL, image_mem));

    VK_ASSERT(vkBindImageMemory(device, *image, *image_mem, 0));
}

void create_sampler(VkDevice device, Texture* textue)
{
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 1000.0f;
    sampler_info.maxAnisotropy = 1.0f;

    VK_ASSERT(
        vkCreateSampler(device, &sampler_info, NULL, &textue->texture_sampler));
}

void copy_buffer_image(VkDevice device, VkCommandPool command_pool, u32 width,
                       u32 height, u32 mip_map_lvl, VkBuffer src_buffer,
                       VkImage dst_image, VkQueue graphics_queue,
                       VkDeviceSize size_bytes)
{
    VkCommandBuffer command_buff = begin_command_buffer(device, command_pool);

    VkImageMemoryBarrier mem_barrier = {};
    mem_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    mem_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    mem_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    mem_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    mem_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    mem_barrier.image = dst_image;
    mem_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    mem_barrier.subresourceRange.baseMipLevel = 0;
    mem_barrier.subresourceRange.levelCount = mip_map_lvl;
    mem_barrier.subresourceRange.baseArrayLayer = 0;
    mem_barrier.subresourceRange.layerCount = 1;
    mem_barrier.srcAccessMask = 0;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(command_buff, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
                         &mem_barrier);

    VkExtent3D image_extent = {};
    image_extent.width = width;
    image_extent.height = height;
    image_extent.depth = 1;

    VkBufferImageCopy img_copy = {};
    img_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_copy.imageSubresource.layerCount = 1;
    img_copy.imageExtent = image_extent;

    vkCmdCopyBufferToImage(command_buff, src_buffer, dst_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &img_copy);

    end_command_buffer(device, command_pool, command_buff, graphics_queue);
}

void enable_bitmap(VkDevice device, VkCommandPool command_pool,
                   VkQueue graphics_queue, VkImage image, const Texture* texture)
{
    VkCommandBuffer command_buff = begin_command_buffer(device, command_pool);

    VkImageMemoryBarrier mem_barrier = {};
    mem_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    mem_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    mem_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    mem_barrier.image = image;
    mem_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    mem_barrier.subresourceRange.levelCount = 1;
    mem_barrier.subresourceRange.baseArrayLayer = 0;
    mem_barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    i32 w = (i32)texture->width;
    i32 h = (i32)texture->height;

    for (u32 i = 1; i < texture->mip_map_lvl; i++)
    {
        // Reset to transfer bit. It will wait for previous. It will be
        // transistion to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL. the
        // last blir or any call before it. In this case is when we copy the
        // buffer to a image. i - 1;
        mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        mem_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        mem_barrier.subresourceRange.baseMipLevel = i - 1;

        vkCmdPipelineBarrier(command_buff, destination_stage, destination_stage, 0,
                             0, NULL, 0, NULL, 1, &mem_barrier);

        VkImageBlit blit = {};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { w, h, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;

        if (w > 1) w /= 2;
        if (h > 1) h /= 2;

        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { w, h, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(command_buff, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                       VK_FILTER_LINEAR);

        mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        mem_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // Wait for the blit command to finish and set it to
        // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
        vkCmdPipelineBarrier(command_buff, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
                             NULL, 1, &mem_barrier);
    }

    // This is for the last mip level. Did not blit i the loop
    mem_barrier.subresourceRange.baseMipLevel = texture->mip_map_lvl - 1;
    mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    mem_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkPipelineStageFlags source_stage = destination_stage;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkCmdPipelineBarrier(command_buff, source_stage, destination_stage, 0, 0, NULL,
                         0, NULL, 1, &mem_barrier);

    end_command_buffer(device, command_pool, command_buff, graphics_queue);
}

u32 rand_rgb(u32 upper, u32 under)
{
    if (upper > 255) upper = 255;

    uint8_t r = (uint8_t)(rand() % (upper - under + 1) + under);

    uint8_t g = (uint8_t)(rand() % (upper - under + 1) + under);

    uint8_t b = (uint8_t)(rand() % (upper - under + 1) + under);

    return (u32)((u32)r | ((u32)g << 8) | ((u32)b << 16));
}

void set_texture_data(VkDevice device, VkPhysicalDevice physical_device, void* data,
                      VkCommandPool command_pool, VkQueue graphics_queue,
                      Texture* texture, VkDeviceSize size_bytes)
{
    Buffer staging_buffer = {};
    staging_buffer.size_bytes = size_bytes;

    helper_buffer(device, physical_device, data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  &staging_buffer);

    copy_buffer_image(device, command_pool, texture->width, texture->height,
                      texture->mip_map_lvl, staging_buffer.buffer, texture->image,
                      graphics_queue, texture->size_bytes);

    destroy_buffer(device, staging_buffer);
}

u32 float_rgba(V4 color)
{
    u8 red = (u8)(color.x * 255.0f);
    u8 green = (u8)(color.y * 255.0f);
    u8 blue = (u8)(color.z * 255.0f);
    u8 alpha = (u8)(color.w * 255.0f);

    return (u32)((alpha << 24) | (blue << 16) | (green << 8) | red);
}

#if 1
static Vec4 pixels_trans(V3 ray_o, V3 ray_dir)
{
    // ray_dir = b
    // ray_o = a
    //(bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
    //
    f32 radius = 0.5f;

    f32 a = v3_dot(ray_dir, ray_dir);
    f32 b = 2.0f * v3_dot(ray_o, ray_dir);
    f32 c = v3_dot(ray_o, ray_o) - (radius * radius);

    // Discriminant
    f32 disc = b * b - 4.0f * a * c;

    if (disc < 0.0f) return v4f(0.0f, 0.0f, 0.0f, 1.0f);

    // f32 t0 = (-b + sqrtf(disc)) / (2.0f * a);
    f32 t1 = (-b - sqrtf(disc)) / (2.0f * a);

    V3 h1 = v3_add(ray_o, v3_s_multi(ray_dir, t1));
    V3 normal = v3_normalize(h1);

    V3 light_dir = v3_normalize(v3f(-1.0f, -1.0f, -1.0f));

    f32 d = maxf32(v3_dot(normal, v3_neg(light_dir)), 0.0f);

    V3 s_color = v3f(1.0f, 0.0f, 1.0f);

    v3_s_multi_equal(&s_color, d);

    return v4f(s_color.x, s_color.y, s_color.z, 1.0f);
}
#endif

// void ray_casting_ex(VkDevice device, VkPhysicalDevice physical_device,
//                     const Camera& camera, VkCommandPool command_pool,
//                     VkQueue graphics_queue, Texture* texture)
//{
//     const u32 width  = texture->width;
//     const u32 height = texture->height;
//     const u32 size   = width * height * 4;
//     u32 pixels[size / 4];
//
//     for (u32 y = 0; y < height; y++)
//     {
//         for (u32 x = 0; x < width; x++)
//         {
//             Vec4 color =
//                 clamp(pixels_trans(camera.position, camera.ray_dirs[x + y *
//                 width]),
//                       Vec4(0.0f), Vec4(1.0f));
//
//             pixels[x + y * width] = float_rgba(color);
//         }
//     }
//
//     texture->size_bytes = size;
//
//     set_texture_data(device, physical_device, pixels, command_pool,
//     graphics_queue,
//                      texture, size);
// }
//

static i32 max_i(i32 f, i32 s)
{
    return (f > s) ? f : s;
}

u32 create_textures_path(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, u32 num_textures, const char** tex_paths,
                         Texture* textures)
{
    for_range(i, num_textures)
    {
        create_texture_path(device, physical_device, command_pool, graphics_queue,
                            mip_map, VK_FORMAT_R8G8B8A8_SRGB, tex_paths[i],
                            textures + i);
    }
    return num_textures;
}

void create_texture_path(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, VkFormat image_format, const char* tex_path,
                         Texture* texture)
{
    i32 w, h, c;
    stbi_uc* tex_buffer = stbi_load(tex_path, &w, &h, &c, STBI_rgb_alpha);

    texture->size_bytes = (u32)w * h * 4;
    texture->width = (u32)w;
    texture->height = (u32)h;
    // Source: vulkan tutorial
    if (mip_map)
    {
        texture->mip_map_lvl = (u32)(floorf(log2f((f32)max_i(w, h)))) + 1;
    }
    else
    {
        texture->mip_map_lvl = 1;
    }

    create_image(texture->width, texture->height, device, physical_device,
                 image_format, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->image,
                 &texture->img_memory, texture->mip_map_lvl, VK_SAMPLE_COUNT_1_BIT);

    set_texture_data(device, physical_device, tex_buffer, command_pool,
                     graphics_queue, texture, texture->size_bytes);

    create_sampler(device, texture);

    create_image_view(device, texture->image, VK_IMAGE_VIEW_TYPE_2D, image_format,
                      VK_IMAGE_ASPECT_COLOR_BIT, texture->mip_map_lvl,
                      &texture->img_view);

    enable_bitmap(device, command_pool, graphics_queue, texture->image, texture);

    stbi_image_free(tex_buffer);
}
void create_texture_buffer(VkDevice device, VkPhysicalDevice physical_device,
                           VkCommandPool command_pool, VkQueue graphics_queue,
                           VkFormat image_format, Texture* texture,
                           unsigned char* tex_buffer)

{
    create_image(texture->width, texture->height, device, physical_device,
                 image_format, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->image,
                 &texture->img_memory, texture->mip_map_lvl, VK_SAMPLE_COUNT_1_BIT);

    set_texture_data(device, physical_device, tex_buffer, command_pool,
                     graphics_queue, texture, texture->size_bytes);

    create_sampler(device, texture);

    create_image_view(device, texture->image, VK_IMAGE_VIEW_TYPE_2D, image_format,
                      VK_IMAGE_ASPECT_COLOR_BIT, texture->mip_map_lvl,
                      &texture->img_view);

#if 1
    enable_bitmap(device, command_pool, graphics_queue, texture->image, texture);
#endif
}

void create_texture(VkDevice device, VkPhysicalDevice physical_device, u32 width,
                    u32 height, VkCommandPool command_pool, VkQueue graphics_queue,
                    Texture* texture)
{
    texture->width = width;
    texture->height = height;

    VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

    create_image(texture->width, texture->height, device, physical_device,
                 image_format, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->image,
                 &texture->img_memory, 1, VK_SAMPLE_COUNT_1_BIT);

    create_sampler(device, texture);

    create_image_view(device, texture->image, VK_IMAGE_VIEW_TYPE_2D, image_format,
                      VK_IMAGE_ASPECT_COLOR_BIT, 1, &texture->img_view);
}

void create_depth_image(VkDevice device, VkPhysicalDevice physical_device,
                        const VkExtent2D* extent_2D,
                        VkSampleCountFlagBits sample_count, Image* depth_image)
{
    VkFormat image_format = VK_FORMAT_D32_SFLOAT;

    create_image(extent_2D->width, extent_2D->height, device, physical_device,
                 image_format, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depth_image->image,
                 &depth_image->img_memory, 1, sample_count);

    create_image_view(device, depth_image->image, VK_IMAGE_VIEW_TYPE_2D,
                      image_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1,
                      &depth_image->img_view);
}
void begin_render_pass(VkCommandBuffer command_buffer, VkRenderPass render_pass,
                       VkFramebuffer framebuffer, const VkExtent2D* extent_2D)
{
    vkResetCommandBuffer(command_buffer, 0);

    VkCommandBufferBeginInfo buffer_begin_info = {};
    buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_ASSERT(vkBeginCommandBuffer(command_buffer, &buffer_begin_info));

    VkClearValue clear_values[2] = {};
    clear_values[0].color.float32[0] = RGB(16.0f);
    clear_values[0].color.float32[1] = RGB(26.0f);
    clear_values[0].color.float32[2] = RGB(3.0f);
    clear_values[0].color.float32[3] = 1.0f;

    clear_values[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo render_pass_begin_info = {};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = render_pass;
    render_pass_begin_info.framebuffer = framebuffer;
    render_pass_begin_info.renderArea.extent.width = extent_2D->width;
    render_pass_begin_info.renderArea.extent.height = extent_2D->height;
    render_pass_begin_info.renderArea.offset = { 0, 0 };
    render_pass_begin_info.clearValueCount = 2;
    render_pass_begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void end_render_pass(VkCommandBuffer command_buffer)
{
    vkCmdEndRenderPass(command_buffer);

    VK_ASSERT(vkEndCommandBuffer(command_buffer));
}

void bind_and_draw_graphics_pipline(VkCommandBuffer command_buffer,
                                    VkDescriptorSet desc_set, u32 index_offset,
                                    u32 index_count,
                                    const Vertex_Buffer& vertex_buffer,
                                    const Index_Buffer& index_buffer,
                                    const Graphic_Pipeline& graphic_pipline)
{
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphic_pipline.pipeline);

    VkDeviceSize offset[] = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer.buffer.buffer,
                           offset);
    vkCmdBindIndexBuffer(command_buffer, index_buffer.buffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            graphic_pipline.layout, 0, 1, &desc_set, 0, NULL);

    vkCmdDrawIndexed(command_buffer, index_count, 1, index_offset, 0, 0);
}

void bind_and_draw_graphics_pipline(VkCommandBuffer command_buffer,
                                    VkDescriptorSet desc_set, u32 index_offset,
                                    u32 index_count,
                                    const Graphic_Pipeline& graphic_pipline)
{
    bind_and_draw_graphics_pipline(command_buffer, desc_set, index_offset,
                                   index_count, graphic_pipline.vert_buffer,
                                   graphic_pipline.idx_buffer, graphic_pipline);
}

void destroy_buffer(VkDevice device, Buffer buffer)
{
    vkFreeMemory(device, buffer.buffer_memory, NULL);
    vkDestroyBuffer(device, buffer.buffer, NULL);
}

void destroy_texture(VkDevice device, Texture texture)
{
    vkDestroyImage(device, texture.image, NULL);
    vkDestroyImageView(device, texture.img_view, NULL);
    vkDestroySampler(device, texture.texture_sampler, NULL);
    vkFreeMemory(device, texture.img_memory, NULL);
}

void destroy_image(VkDevice device, Image image)
{
    vkDestroyImage(device, image.image, NULL);
    vkDestroyImageView(device, image.img_view, NULL);
    vkFreeMemory(device, image.img_memory, NULL);
}

void copy_data_buffer(Buffer* buffer, void* data, size_t size_bytes)
{
    memcpy(buffer->transfer_data, data, size_bytes);
}

void update_buffers(VkDevice device, Buffer* buffer, void* data, size_t size_bytes)
{
    buffer->transfer_data = NULL;
    vkMapMemory(device, buffer->buffer_memory, 0, sizeof(MVP), 0,
                &buffer->transfer_data);
    memcpy(buffer->transfer_data, data, size_bytes);
    vkUnmapMemory(device, buffer->buffer_memory);
}

