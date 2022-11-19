#include "buffers.h"
#include "logging.h"
#include "stb/stb_image.h"
#include "camera.h"
#include "region_alloc.h"
#include <string.h>
#include <math.h>

namespace synt {

// TODO: Need to fix this
void create_image_view(VkDevice device, VkImage image,
                       VkImageViewType image_view_type, VkFormat image_format,
                       VkImageAspectFlags aspect_mask, uint32 mip_map_lvl,
                       VkImageView* image_view);

#define RGB(x) x / 255.0f

static int32 get_type_index(VkPhysicalDeviceMemoryProperties mem_props,
                            VkMemoryRequirements mem_req,
                            VkMemoryPropertyFlags wanted_mem_props)
{
    for (uint32 i = 0; i < mem_props.memoryTypeCount; i++)
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
    INIT_0(VkBufferCreateInfo, buffer_info);
    buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size        = data_size;
    buffer_info.usage       = usage_flags;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_ASSERT(vkCreateBuffer(device, &buffer_info, NULL, buffer));

    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(device, *buffer, &mem_req);

    int32 mem_type_idx = get_type_index(mem_props, mem_req, wanted_mem_props);
    assert(mem_type_idx != -1);

    INIT_0(VkMemoryAllocateInfo, alloc_info);
    alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize  = mem_req.size;
    alloc_info.memoryTypeIndex = (uint32)mem_type_idx;

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

    if (data != NULL)
    {
        map_copy_mem(device, &buffer->buffer_memory, buffer->size_bytes, data);
    }
}

static void staging_buffers(VkDevice device, VkPhysicalDevice physical_device,
                            VkCommandPool command_pool, VkQueue graphics_queue,
                            VkBufferUsageFlags vertex_or_index, void* data,
                            VkBuffer* buffer, VkDeviceMemory* buffer_memory,
                            VkDeviceSize size_bytes)
{
    INIT_0(Buffer, staging_buffer);
    staging_buffer.size_bytes = size_bytes;
    assert(staging_buffer.size_bytes);

    helper_buffer(device, physical_device, data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  &staging_buffer);

    create_alloc_bind(device, physical_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      vertex_or_index | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer,
                      buffer_memory, size_bytes);

    copy_buffer(device, command_pool, staging_buffer.buffer, *buffer, graphics_queue,
                size_bytes);

    destroy_buffer(device, staging_buffer.buffer, staging_buffer.buffer_memory);
}

VkCommandBuffer begin_command_buffer(VkDevice device, VkCommandPool command_pool)
{
    VkCommandBuffer command_buff = VK_NULL_HANDLE;
    allocate_commandbuffer(device, command_pool, &command_buff);

    INIT_0(VkCommandBufferBeginInfo, begin_info);
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    vkBeginCommandBuffer(command_buff, &begin_info);

    return command_buff;
}

void end_command_buffer(VkDevice device, VkCommandPool command_pool,
                        VkCommandBuffer command_buff, VkQueue graphics_queue)
{
    vkEndCommandBuffer(command_buff);

    INIT_0(VkSubmitInfo, submitInfo);
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &command_buff;

    vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buff);
}

void copy_buffer(VkDevice device, VkCommandPool command_pool, VkBuffer src_buffer,
                 VkBuffer dst_buffer, VkQueue graphics_queue,
                 VkDeviceSize size_bytes)
{
    VkCommandBuffer command_buff = begin_command_buffer(device, command_pool);

    INIT_0(VkBufferCopy, buff_copy);
    buff_copy.size = size_bytes;
    vkCmdCopyBuffer(command_buff, src_buffer, dst_buffer, 1, &buff_copy);

    end_command_buffer(device, command_pool, command_buff, graphics_queue);
}

void map_copy_mem(VkDevice device, VkDeviceMemory* buffer_memory,
                  VkDeviceSize size_bytes, void* data)
{
    void* transfer_data = NULL;
    VK_ASSERT(vkMapMemory(device, *buffer_memory, 0, size_bytes, 0, &transfer_data));
    memcpy(transfer_data, data, (size_t)size_bytes);
    vkUnmapMemory(device, *buffer_memory);
}

void create_vertex_buffer(VkDevice device, VkPhysicalDevice physical_device,
                          VkCommandPool command_pool, VkQueue graphics_queue,
                          Vertex_Buffer* vertex_buffer)
{
    staging_buffers(device, physical_device, command_pool, graphics_queue,
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_buffer->data,
                    &vertex_buffer->buffer, &vertex_buffer->buffer_memory,
                    vertex_buffer->size_bytes);
}

void create_index_buffer(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         Index_Buffer* index_buffer)
{
    staging_buffers(device, physical_device, command_pool, graphics_queue,
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_buffer->data,
                    &index_buffer->buffer, &index_buffer->buffer_memory,
                    index_buffer->size_bytes);
}

void create_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device,
                           Uniform_Buffer* uniform_buffer)
{
    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &uniform_buffer->buffer,
                      &uniform_buffer->buffer_memory, uniform_buffer->size_bytes);
}

void create_command_pool(VkDevice device, uint32 queue_fam_index,
                         VkCommandPool* command_pool)
{
    INIT_0(VkCommandPoolCreateInfo, create_info);
    create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = queue_fam_index;
    create_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    *command_pool = VK_NULL_HANDLE;
    VK_ASSERT(vkCreateCommandPool(device, &create_info, NULL, command_pool));
}

void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                            VkCommandBuffer* command_buffer)
{
    INIT_0(VkCommandBufferAllocateInfo, alloc_info);
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = command_pool;
    alloc_info.commandBufferCount = 1;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_ASSERT(vkAllocateCommandBuffers(device, &alloc_info, command_buffer));
}

void update_descritors(Region_Alloc* region, VkDevice device,
                       Descriptors* desciptors, uint32 desc_count,
                       const Texture* textures, uint32 num_textures,
                       Uniform_Buffer* uniform_buffers)
{
    for (uint32 i = 0; i < desc_count; i++)
    {
        INIT_0(VkDescriptorBufferInfo, buffer_info);
        buffer_info.buffer = uniform_buffers[i].buffer;
        buffer_info.range  = sizeof(MVP);

        Temp_Alloc<VkDescriptorImageInfo> image_infos(region, num_textures);
        for (uint32 j = 0; j < num_textures; j++)
        {
            INIT_0(VkDescriptorImageInfo, image_info);
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView   = textures[j].img_view;
            image_info.sampler     = textures[j].texture_sampler;

            image_infos.push_back(image_info);
        }

        INIT_ARR0(VkWriteDescriptorSet, desc_writes, 2);
        desc_writes[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        desc_writes[0].pBufferInfo     = &buffer_info;
        desc_writes[0].dstSet          = desciptors->desc_sets[i];
        desc_writes[0].dstBinding      = 0;

        desc_writes[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_writes[1].descriptorCount = image_infos.size();
        desc_writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        desc_writes[1].pImageInfo      = image_infos.data;
        desc_writes[1].dstSet          = desciptors->desc_sets[i];
        desc_writes[1].dstBinding      = 1;

        vkUpdateDescriptorSets(device, sy_SIZE(desc_writes), desc_writes, 0, NULL);
    }
}

void create_descriptors(Region_Alloc* region, VkDevice device,
                        Descriptors* desciptors, uint32 desc_count,
                        VkDescriptorSetLayout desc_layout, const Texture* texture,
                        uint32 num_textures, Uniform_Buffer* uniform_buffers)
{
    desciptors->desc_count = desc_count;

    INIT_ARR0(VkDescriptorPoolSize, pool_sizes, 2);
    pool_sizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = desc_count;

    pool_sizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = desc_count * num_textures;

    INIT_0(VkDescriptorPoolCreateInfo, pool_info);
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets       = desc_count;
    pool_info.poolSizeCount = sy_SIZE(pool_sizes);
    pool_info.pPoolSizes    = pool_sizes;

    VK_ASSERT(
        vkCreateDescriptorPool(device, &pool_info, NULL, &desciptors->desc_pool));

    if (!desciptors->desc_sets) ERROR("Need to allocate descriptor sets");

    VkDescriptorSetLayout set_layout[] = { desc_layout, desc_layout };

    INIT_0(VkDescriptorSetAllocateInfo, alloc_info);
    alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool     = desciptors->desc_pool;
    alloc_info.descriptorSetCount = desc_count;
    alloc_info.pSetLayouts        = set_layout;

    VK_ASSERT(vkAllocateDescriptorSets(device, &alloc_info, desciptors->desc_sets));

    update_descritors(region, device, desciptors, desc_count, texture, num_textures,
                      uniform_buffers);
}

void create_image(uint32_t width, uint32_t height, VkDevice device,
                  VkPhysicalDevice physical_device, VkFormat format,
                  VkImageTiling tiling, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags wanted_mem_props, VkImage* image,
                  VkDeviceMemory* image_mem, uint32_t mip_map_lvl,
                  VkSampleCountFlagBits num_samples)
{

    INIT_0(VkImageCreateInfo, image_info);
    image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType     = VK_IMAGE_TYPE_2D;
    image_info.extent.width  = width;
    image_info.extent.height = height;
    image_info.extent.depth  = 1;
    image_info.mipLevels     = mip_map_lvl;
    image_info.arrayLayers   = 1;
    image_info.format        = format;
    image_info.tiling        = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage         = usage;
    image_info.samples       = num_samples;
    image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    VK_ASSERT(vkCreateImage(device, &image_info, NULL, image));

    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(device, *image, &mem_req);

    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    int32 mem_type_idx = get_type_index(mem_props, mem_req, wanted_mem_props);
    assert(mem_type_idx != -1);

    INIT_0(VkMemoryAllocateInfo, mem_alloc_info);
    mem_alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc_info.allocationSize  = mem_req.size;
    mem_alloc_info.memoryTypeIndex = (uint32)mem_type_idx;

    VK_ASSERT(vkAllocateMemory(device, &mem_alloc_info, NULL, image_mem));

    VK_ASSERT(vkBindImageMemory(device, *image, *image_mem, 0));
}

void create_sampler(VkDevice device, Texture* textue)
{
    INIT_0(VkSamplerCreateInfo, sampler_info);
    sampler_info.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter     = VK_FILTER_LINEAR;
    sampler_info.minFilter     = VK_FILTER_LINEAR;
    sampler_info.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.minLod        = 0.0f;
    sampler_info.maxLod        = 1000.0f;
    sampler_info.maxAnisotropy = 1.0f;

    VK_ASSERT(
        vkCreateSampler(device, &sampler_info, NULL, &textue->texture_sampler));
}

void copy_buffer_image(VkDevice device, VkCommandPool command_pool, uint32 width,
                       uint32 height, uint32 mip_map_lvl, VkBuffer src_buffer,
                       VkImage dst_image, VkQueue graphics_queue,
                       VkDeviceSize size_bytes)
{
    VkCommandBuffer command_buff = begin_command_buffer(device, command_pool);

    INIT_0(VkImageMemoryBarrier, mem_barrier);
    mem_barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    mem_barrier.oldLayout                   = VK_IMAGE_LAYOUT_UNDEFINED;
    mem_barrier.newLayout                   = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    mem_barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    mem_barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    mem_barrier.image                       = dst_image;
    mem_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    mem_barrier.subresourceRange.baseMipLevel   = 0;
    mem_barrier.subresourceRange.levelCount     = mip_map_lvl;
    mem_barrier.subresourceRange.baseArrayLayer = 0;
    mem_barrier.subresourceRange.layerCount     = 1;
    mem_barrier.srcAccessMask                   = 0;
    mem_barrier.dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(command_buff, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
                         &mem_barrier);

    INIT_0(VkExtent3D, image_extent);
    image_extent.width  = width;
    image_extent.height = height;
    image_extent.depth  = 1;

    INIT_0(VkBufferImageCopy, img_copy);
    img_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_copy.imageSubresource.layerCount = 1;
    img_copy.imageExtent                 = image_extent;

    vkCmdCopyBufferToImage(command_buff, src_buffer, dst_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &img_copy);

    end_command_buffer(device, command_pool, command_buff, graphics_queue);
}

void enable_bitmap(VkDevice device, VkCommandPool command_pool,
                   VkQueue graphics_queue, VkImage image, const Texture& texture)
{
    VkCommandBuffer command_buff = begin_command_buffer(device, command_pool);

    INIT_0(VkImageMemoryBarrier, mem_barrier);
    mem_barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    mem_barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    mem_barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    mem_barrier.image                       = image;
    mem_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    mem_barrier.subresourceRange.levelCount = 1;
    mem_barrier.subresourceRange.baseArrayLayer = 0;
    mem_barrier.subresourceRange.layerCount     = 1;

    VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    int32 w = (int32)texture.width;
    int32 h = (int32)texture.height;

    for (uint32 i = 1; i < texture.mip_map_lvl; i++)
    {
        // Reset to transfer bit. It will wait for previous. It will be
        // transistion to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL. the
        // last blir or any call before it. In this case is when we copy the
        // buffer to a image. i - 1;
        mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        mem_barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        mem_barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        mem_barrier.subresourceRange.baseMipLevel = i - 1;

        vkCmdPipelineBarrier(command_buff, destination_stage, destination_stage, 0,
                             0, NULL, 0, NULL, 1, &mem_barrier);

        INIT_0(VkImageBlit, blit);
        blit.srcOffsets[0]                 = { 0, 0, 0 };
        blit.srcOffsets[1]                 = { w, h, 1 };
        blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel       = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount     = 1;

        if (w > 1) w /= 2;
        if (h > 1) h /= 2;

        blit.dstOffsets[0]                 = { 0, 0, 0 };
        blit.dstOffsets[1]                 = { w, h, 1 };
        blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel       = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount     = 1;

        vkCmdBlitImage(command_buff, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                       VK_FILTER_LINEAR);

        mem_barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        mem_barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // Wait for the blit command to finish and set it to
        // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
        vkCmdPipelineBarrier(command_buff, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
                             NULL, 1, &mem_barrier);
    }

    // This is for the last mip level. Did not blit i the loop
    mem_barrier.subresourceRange.baseMipLevel = texture.mip_map_lvl - 1;
    mem_barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    mem_barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkPipelineStageFlags source_stage = destination_stage;
    destination_stage                 = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkCmdPipelineBarrier(command_buff, source_stage, destination_stage, 0, 0, NULL,
                         0, NULL, 1, &mem_barrier);

    end_command_buffer(device, command_pool, command_buff, graphics_queue);
}

uint32_t rand_rgb(uint32_t upper, uint32_t under)
{
    if (upper > 255) upper = 255;

    uint8_t r = (uint8_t)(rand() % (upper - under + 1) + under);

    uint8_t g = (uint8_t)(rand() % (upper - under + 1) + under);

    uint8_t b = (uint8_t)(rand() % (upper - under + 1) + under);

    return (uint32_t)((uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16));
}

void set_texture_data(VkDevice device, VkPhysicalDevice physical_device, void* data,
                      VkCommandPool command_pool, VkQueue graphics_queue,
                      Texture* texture, VkDeviceSize size_bytes)
{
    INIT_0(Buffer, staging_buffer);
    staging_buffer.size_bytes = size_bytes;

    helper_buffer(device, physical_device, data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  &staging_buffer);

    copy_buffer_image(device, command_pool, texture->width, texture->height,
                      texture->mip_map_lvl, staging_buffer.buffer, texture->image,
                      graphics_queue, texture->size_bytes);

    destroy_buffer(device, staging_buffer.buffer, staging_buffer.buffer_memory);
}

static uint32 float_rgba(const Vec4& color)
{
    uint8 red   = (uint8)(color.x * 255.0f);
    uint8 green = (uint8)(color.y * 255.0f);
    uint8 blue  = (uint8)(color.z * 255.0f);
    uint8 alpha = (uint8)(color.w * 255.0f);

    return (uint32)((alpha << 24) | (blue << 16) | (green << 8) | red);
}

static Vec4 pixels_trans(const Vec3& ray_o, const Vec3& ray_dir)
{
    //(bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
    //
    float radius = 0.5f;

    float a = dot(ray_dir, ray_dir);
    float b = 2.0f * dot(ray_o, ray_dir);
    float c = dot(ray_o, ray_o) - (radius * radius);

    // Discriminant
    float disc = b * b - 4.0f * a * c;

    if (disc < 0.0f) return Vec4(0.0f, 0.0f, 0.0f, 1.0f);

    float t0 = (-b + sqrt(disc)) / (2.0f * a);
    float t1 = (-b - sqrt(disc)) / (2.0f * a);

    Vec3 h1     = ray_o + ray_dir * t1;
    Vec3 normal = normalize(h1);

    Vec3 light_dir = normalize(Vec3(-1.0f, -1.0f, -1.0f));

    float d = maxf32(dot(normal, -1.0f * light_dir), 0.0f);

    Vec3 s_color(1.0f, 0.0f, 1.0f);

    s_color *= d;

    return Vec4(s_color.x, s_color.y, s_color.z, 1.0f);
}

// void ray_casting_ex(VkDevice device, VkPhysicalDevice physical_device,
//                     const Camera& camera, VkCommandPool command_pool,
//                     VkQueue graphics_queue, Texture* texture)
//{
//     const uint32 width  = texture->width;
//     const uint32 height = texture->height;
//     const uint32 size   = width * height * 4;
//     uint32 pixels[size / 4];
//
//     for (uint32 y = 0; y < height; y++)
//     {
//         for (uint32 x = 0; x < width; x++)
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

static int32 max(int32 f, int32 s) { return (f > s) ? f : s; }

void create_texture(VkDevice device, VkPhysicalDevice physical_device,
                    VkCommandPool command_pool, VkQueue graphics_queue, bool mip_map,
                    VkFormat image_format, const char* tex_path, Texture* texture)
{
    int w, h, c;
    stbi_uc* tex_buffer = stbi_load(tex_path, &w, &h, &c, STBI_rgb_alpha);

    texture->size_bytes = (uint32)w * h * 4;
    texture->width      = (uint32)w;
    texture->height     = (uint32)h;
    // Source: vulkan tutorial
    if (mip_map)
    {
        texture->mip_map_lvl = (uint32)(std::floor(std::log2(max(w, h)))) + 1;
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

    enable_bitmap(device, command_pool, graphics_queue, texture->image, *texture);

    stbi_image_free(tex_buffer);
}
void create_texture(VkDevice device, VkPhysicalDevice physical_device,
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
    enable_bitmap(device, command_pool, graphics_queue, texture->image, *texture);
#endif
}

void create_texture(VkDevice device, VkPhysicalDevice physical_device, uint32 width,
                    uint32 height, VkCommandPool command_pool,
                    VkQueue graphics_queue, Texture* texture)
{
    texture->width  = width;
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
                        VkExtent2D extent_2D, VkSampleCountFlagBits sample_count,
                        Image* depth_image)
{
    VkFormat image_format = VK_FORMAT_D32_SFLOAT;

    create_image(extent_2D.width, extent_2D.height, device, physical_device,
                 image_format, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depth_image->image,
                 &depth_image->img_memory, 1, sample_count);

    create_image_view(device, depth_image->image, VK_IMAGE_VIEW_TYPE_2D,
                      image_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1,
                      &depth_image->img_view);
}
void begin_render_pass(VkCommandBuffer command_buffer, VkRenderPass render_pass,
                       VkFramebuffer framebuffer, VkExtent2D extent_2D)
{
    vkResetCommandBuffer(command_buffer, 0);

    INIT_0(VkCommandBufferBeginInfo, buffer_begin_info)
    buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_ASSERT(vkBeginCommandBuffer(command_buffer, &buffer_begin_info));

    INIT_ARR0(VkClearValue, clear_values, 2);
    clear_values[0].color.float32[0] = RGB(16.0f);
    clear_values[0].color.float32[1] = RGB(26.0f);
    clear_values[0].color.float32[2] = RGB(3.0f);
    clear_values[0].color.float32[3] = 1.0f;

    clear_values[1].depthStencil = { 1.0f, 0 };

    INIT_0(VkRenderPassBeginInfo, render_pass_begin_info);
    render_pass_begin_info.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass  = render_pass;
    render_pass_begin_info.framebuffer = framebuffer;
    render_pass_begin_info.renderArea.extent = extent_2D;
    render_pass_begin_info.renderArea.offset = (VkOffset2D){ 0, 0 };
    render_pass_begin_info.clearValueCount   = 2;
    render_pass_begin_info.pClearValues      = clear_values;

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void end_render_pass(VkCommandBuffer command_buffer)
{
    vkCmdEndRenderPass(command_buffer);

    VK_ASSERT(vkEndCommandBuffer(command_buffer));
}

void bind_and_draw_graphics_pipline(VkCommandBuffer command_buffer,
                                    VkDescriptorSet desc_set,
                                    const Graphic_Pipline& graphic_pipline,
                                    bool if_desc_set) // TODO: bool quick solution
{
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphic_pipline.pipeline);

    VkDeviceSize offset[] = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &graphic_pipline.vert_buffer.buffer,
                           offset);
    vkCmdBindIndexBuffer(command_buffer, graphic_pipline.idx_buffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);
    if (if_desc_set)
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                graphic_pipline.layout, 0, 1, &desc_set, 0, NULL);

    uint32 index_count = graphic_pipline.idx_buffer.curr_size;

    vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
}

void destroy_buffer(VkDevice device, VkBuffer buffer, VkDeviceMemory buffer_memory)
{
    vkFreeMemory(device, buffer_memory, NULL);
    vkDestroyBuffer(device, buffer, NULL);
}

void destroy_texture(VkDevice device, Texture& texture)
{
    vkDestroyImage(device, texture.image, NULL);
    vkDestroyImageView(device, texture.img_view, NULL);
    vkDestroySampler(device, texture.texture_sampler, NULL);
    vkFreeMemory(device, texture.img_memory, NULL);

    texture.image           = VK_NULL_HANDLE;
    texture.img_memory      = VK_NULL_HANDLE;
    texture.img_view        = VK_NULL_HANDLE;
    texture.texture_sampler = VK_NULL_HANDLE;
    texture.width           = 0;
    texture.height          = 0;
    texture.size_bytes      = 0;
}

void destroy_image(VkDevice device, Image& image)
{
    vkDestroyImage(device, image.image, NULL);
    vkDestroyImageView(device, image.img_view, NULL);
    vkFreeMemory(device, image.img_memory, NULL);

    image.image      = VK_NULL_HANDLE;
    image.img_memory = VK_NULL_HANDLE;
    image.img_view   = VK_NULL_HANDLE;
}

Rect quad(Vertex** vertices, const Vec3& pos, const Vec2& size, const Vec4& color,
          float tex_index)
{
    Vertex verts[4] = { { { pos.x, pos.y, pos.z },
                          { color.x, color.y, color.z, color.w },
                          { 0.0f, 0.0f },
                          tex_index },
                        { { pos.x, pos.y + size.y, pos.z },
                          { color.x, color.y, color.z, color.w },
                          { 0.0f, 1.0f },
                          tex_index },
                        { { pos.x + size.x, pos.y + size.y, pos.z },
                          { color.x, color.y, color.z, color.w },
                          { 1.0f, 1.0f },
                          tex_index },
                        { { pos.x + size.x, pos.y, pos.z },
                          { color.x, color.y, color.z, color.w },
                          { 1.0f, 0.0f },
                          tex_index } };

    for (uint32 i = 0; i < 4; i++)
    {
        synt_push((*vertices), verts[i]);
    }

    Rect out;
    out.pos.x = pos.x;
    out.pos.y = pos.y;
    out.size  = size;
    return out;
}

void update_uniform_buffers(VkDevice device, const Uniform_Buffer& uniform_buffer,
                            void* data, size_t size_bytes)
{
    void* transer_data;
    vkMapMemory(device, uniform_buffer.buffer_memory, 0, sizeof(MVP), 0,
                &transer_data);
    memcpy(transer_data, data, size_bytes);
    vkUnmapMemory(device, uniform_buffer.buffer_memory);
}

} // namespace synt
