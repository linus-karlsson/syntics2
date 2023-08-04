
i32 type_index_get(VkPhysicalDeviceMemoryProperties mem_props,
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

void mem_map_copy(VkDevice device, Buffer* buffer, void* data)
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

void mem_map_copy_index(VkDevice device, Index_Buffer* ib)
{
    ib->array.data = NULL;
    if (vkMapMemory(device, ib->buffer.buffer_memory, 0, ib->buffer.size_bytes, 0,
                    (void**)&ib->array.data))
    {
        SY_ERROR("vkMapMemory failed\n");
    }
}

void mem_map_copy_vertex(VkDevice device, Vertex_Buffer* vb)
{
    vb->array.data = NULL;
    if (vkMapMemory(device, vb->buffer.buffer_memory, 0, vb->buffer.size_bytes, 0,
                    (void**)&vb->array.data))
    {
        SY_ERROR("vkMapMemory failed\n");
    }
}

void mem_map_copy_unmap(VkDevice device, Buffer* buffer, void* data)
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

void memory_allocate(VkDevice device, VkPhysicalDevice physical_device,
                     VkMemoryRequirements mem_req,
                     VkMemoryPropertyFlags wanted_mem_props, VkDeviceMemory* memory)
{
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    i32 mem_type_idx = type_index_get(mem_props, mem_req, wanted_mem_props);
    assert(mem_type_idx != -1);

    VkMemoryAllocateInfo mem_alloc_info = { 0 };
    mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc_info.allocationSize = mem_req.size;
    mem_alloc_info.memoryTypeIndex = (u32)mem_type_idx;

    VK_ASSERT(vkAllocateMemory(device, &mem_alloc_info, NULL, memory));
}

void commandbuffers_allocate(VkDevice device, VkCommandPool command_pool,
                             VkCommandBufferLevel level, u32 command_buffer_count,
                             VkCommandBuffer* command_buffer)
{
    VkCommandBufferAllocateInfo alloc_info = { 0 };
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = command_buffer_count;
    alloc_info.level = level;

    VK_ASSERT(vkAllocateCommandBuffers(device, &alloc_info, command_buffer));
}

void buffer_destroy(VkDevice device, Buffer buffer)
{
    vkFreeMemory(device, buffer.buffer_memory, NULL);
    vkDestroyBuffer(device, buffer.buffer, NULL);
}

void texture_destroy(VkDevice device, Texture texture)
{
    vkDestroyImage(device, texture.image, NULL);
    vkDestroyImageView(device, texture.img_view, NULL);
    vkDestroySampler(device, texture.texture_sampler, NULL);
    vkFreeMemory(device, texture.img_memory, NULL);
}

void image_destroy(VkDevice device, Image image)
{
    vkDestroyImage(device, image.image, NULL);
    vkDestroyImageView(device, image.img_view, NULL);
    vkFreeMemory(device, image.img_memory, NULL);
}

void buffers_update(VkDevice device, Buffer* buffer, void* data, size_t size_bytes)
{
    buffer->transfer_data = NULL;
    vkMapMemory(device, buffer->buffer_memory, 0, sizeof(VP), 0,
                &buffer->transfer_data);
    memcpy(buffer->transfer_data, data, size_bytes);
    vkUnmapMemory(device, buffer->buffer_memory);
}

VkCommandBuffer command_buffer_begin(VkDevice device, VkCommandPool command_pool,
                                     VkCommandBufferLevel level)
{
    VkCommandBuffer command_buff = VK_NULL_HANDLE;
    commandbuffers_allocate(device, command_pool, level, 1, &command_buff);

    VkCommandBufferBeginInfo begin_info = { 0 };
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    vkBeginCommandBuffer(command_buff, &begin_info);

    return command_buff;
}

void command_buffer_end(VkDevice device, VkCommandPool command_pool,
                        VkCommandBuffer command_buff, VkQueue graphics_queue)
{
    vkEndCommandBuffer(command_buff);

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buff;

    vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buff);
}

void buffer_copy(VkDevice device, VkCommandPool command_pool, VkBuffer src_buffer,
                 VkBuffer dst_buffer, VkQueue graphics_queue,
                 VkDeviceSize size_bytes)
{
    VkCommandBuffer command_buff =
        command_buffer_begin(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkBufferCopy buff_copy = { 0 };
    buff_copy.size = size_bytes;
    vkCmdCopyBuffer(command_buff, src_buffer, dst_buffer, 1, &buff_copy);

    command_buffer_end(device, command_pool, command_buff, graphics_queue);
}

void create_alloc_bind(VkDevice device, VkPhysicalDevice physical_device,
                       VkMemoryPropertyFlags wanted_mem_props,
                       VkBufferUsageFlags usage_flags, VkBuffer* buffer,
                       VkDeviceMemory* buffer_memory, VkDeviceSize data_size)
{
    VkBufferCreateInfo buffer_info = { 0 };
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = data_size;
    buffer_info.usage = usage_flags;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_ASSERT(vkCreateBuffer(device, &buffer_info, NULL, buffer));

    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(device, *buffer, &mem_req);

    memory_allocate(device, physical_device, mem_req, wanted_mem_props,
                    buffer_memory);

    VK_ASSERT(vkBindBufferMemory(device, *buffer, *buffer_memory, 0));
}

void helper_buffer(VkDevice device, VkPhysicalDevice physical_device, void* data,
                   VkBufferUsageFlags usage_flags, Buffer* buffer)
{
    create_alloc_bind(
        device, physical_device,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        usage_flags, &buffer->buffer, &buffer->buffer_memory, buffer->size_bytes);

    mem_map_copy_unmap(device, buffer, data);
}

void staging_buffers(VkDevice device, VkPhysicalDevice physical_device,
                     VkCommandPool command_pool, VkQueue graphics_queue,
                     VkBufferUsageFlags vertex_or_index, void* data,
                     VkBuffer* buffer, VkDeviceMemory* buffer_memory,
                     VkDeviceSize size_bytes)
{
    Buffer staging_buffer = { 0 };
    staging_buffer.size_bytes = size_bytes;
    ASSERT(staging_buffer.size_bytes, "");
    ASSERT(data, "data is null");

    helper_buffer(device, physical_device, data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  &staging_buffer);

    create_alloc_bind(device, physical_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      vertex_or_index | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer,
                      buffer_memory, size_bytes);

    buffer_copy(device, command_pool, staging_buffer.buffer, *buffer, graphics_queue,
                size_bytes);

    buffer_destroy(device, staging_buffer);
}

void vertex_buffer_create_test(VkDevice device, VkPhysicalDevice physical_device,
                               Vertex_Buffer* vertex_buffer)
{
    Buffer* b = &vertex_buffer->buffer;
    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &b->buffer,
                      &b->buffer_memory, b->size_bytes);

    mem_map_copy_vertex(device, vertex_buffer);
}

void vertex_buffer_create_visible(VkDevice device, VkPhysicalDevice physical_device,
                                  Vertex_Buffer* vertex_buffer)
{
    Buffer* b = &vertex_buffer->buffer;
    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &b->buffer,
                      &b->buffer_memory, b->size_bytes);

    mem_map_copy(device, b, vertex_buffer->array.data);
}

void vertex_buffer_create_local(VkDevice device, VkPhysicalDevice physical_device,
                                VkCommandPool command_pool, VkQueue graphics_queue,
                                Vertex_Buffer* vertex_buffer)
{
    Buffer* b = &vertex_buffer->buffer;
    staging_buffers(device, physical_device, command_pool, graphics_queue,
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_buffer->array.data,
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

    mem_map_copy_index(device, index_buffer);
}

void index_buffer_create_visible(VkDevice device, VkPhysicalDevice physical_device,
                                 Index_Buffer* index_buffer)
{
    Buffer* b = &index_buffer->buffer;
    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &b->buffer,
                      &b->buffer_memory, b->size_bytes);

    mem_map_copy(device, b, index_buffer->array.data);
}

void index_buffer_create_local(VkDevice device, VkPhysicalDevice physical_device,
                               VkCommandPool command_pool, VkQueue graphics_queue,
                               Index_Buffer* index_buffer)
{
    Buffer* b = &index_buffer->buffer;
    staging_buffers(device, physical_device, command_pool, graphics_queue,
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_buffer->array.data,
                    &b->buffer, &b->buffer_memory, b->size_bytes);
}

void vertex_index_buffer_create_default(
    VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool,
    VkQueue graphics_queue, Visible_Local visible_local,
    Vertex_Buffer* vertex_buffer, Index_Buffer* index_buffer)
{
    vertex_buffer->buffer.size_bytes =
        vertex_buffer->array._capacity * sizeof(Vertex);
    index_buffer->buffer.size_bytes = index_buffer->array._capacity * sizeof(u32);

    switch (visible_local)
    {
        case VERTEX_INDEX_VISIBLE_VISIBLE:
        {
            vertex_buffer_create_visible(device, physical_device, vertex_buffer);
            index_buffer_create_visible(device, physical_device, index_buffer);
            break;
        }
        case VERTEX_INDEX_VISIBLE_LOCAL:
        {
            vertex_buffer_create_visible(device, physical_device, vertex_buffer);
            index_buffer_create_local(device, physical_device, command_pool,
                                      graphics_queue, index_buffer);
            break;
        }
        case VERTEX_INDEX_LOCAL_VISIBLE:
        {
            vertex_buffer_create_local(device, physical_device, command_pool,
                                       graphics_queue, vertex_buffer);
            index_buffer_create_visible(device, physical_device, index_buffer);
            break;
        }
        case VERTEX_INDEX_LOCAL_LOCAL:
        {
            vertex_buffer_create_local(device, physical_device, command_pool,
                                       graphics_queue, vertex_buffer);
            index_buffer_create_local(device, physical_device, command_pool,
                                      graphics_queue, index_buffer);
            break;
        }
    }
}

void vertex_index_buffer_create_default1(VkDevice device,
                                         VkPhysicalDevice physical_device,
                                         VkCommandPool command_pool,
                                         VkQueue graphics_queue,
                                         Visible_Local visible_local,
                                         Vertex_Index_Buffer* vertex_index_buffer)
{
    vertex_index_buffer_create_default(
        device, physical_device, command_pool, graphics_queue, visible_local,
        &vertex_index_buffer->vert, &vertex_index_buffer->idx);
}

void uniform_buffer_create(VkDevice device, VkPhysicalDevice physical_device,
                           Uniform_Buffer* uniform_buffer)
{
    Buffer* b = &uniform_buffer->buffer;
    create_alloc_bind(device, physical_device,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &b->buffer,
                      &b->buffer_memory, b->size_bytes);

    b->transfer_data = NULL;
    VK_ASSERT(vkMapMemory(device, b->buffer_memory, 0, b->size_bytes, 0,
                          &b->transfer_data));
}

void command_pool_create(VkDevice device, u32 queue_fam_index,
                         VkCommandPool* command_pool)
{
    VkCommandPoolCreateInfo create_info = { 0 };
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = queue_fam_index;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    *command_pool = VK_NULL_HANDLE;
    VK_ASSERT(vkCreateCommandPool(device, &create_info, NULL, command_pool));
}

void update_descritors(Region_Alloc* region, VkDevice device,
                       Descriptors* desciptors, u32 desc_count,
                       const Texture* textures, u32 num_textures,
                       Uniform_Buffer* uniform_buffers)
{
    stack_begin_scope(desc_stack);

    for (u32 i = 0; i < desc_count; i++)
    {
#if 1
        VkDescriptorBufferInfo buffer_info = { 0 };
        buffer_info.buffer = uniform_buffers[i].buffer.buffer;
        buffer_info.range = sizeof(VP);
#endif

        VkDescriptorImageInfo* image_infos =
            stack_malloc(num_textures, VkDescriptorImageInfo);

        for (u32 j = 0; j < num_textures; j++)
        {
            VkDescriptorImageInfo image_info = { 0 };
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView = textures[j].img_view;
            image_info.sampler = textures[j].texture_sampler;

            image_infos[j] = image_info;
        }

#if 1
        VkWriteDescriptorSet desc_writes[2] = { 0 };
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

    stack_end_scope(desc_stack);
}

void descriptors_create(Region_Alloc* region, VkDevice device,
                        Descriptors* desciptors, u32 desc_count,
                        VkDescriptorSetLayout desc_layout, const Texture* texture,
                        u32 num_textures, Uniform_Buffer* uniform_buffers)
{
    stack_begin_scope(desc_stack);

    desciptors->desc_count = desc_count;

#if 1
    VkDescriptorPoolSize pool_sizes[2] = { 0 };
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = desc_count;

    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = desc_count * num_textures;
#else
    VkDescriptorPoolSize pool_sizes[1] = { 0 };
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[0].descriptorCount = desc_count * num_textures;
#endif

    VkDescriptorPoolCreateInfo pool_info = { 0 };
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = desc_count;
    pool_info.poolSizeCount = sy_SIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VK_ASSERT(
        vkCreateDescriptorPool(device, &pool_info, NULL, &desciptors->desc_pool));

    if (!desciptors->desc_sets) SY_ERROR("Need to allocate descriptor sets");

    VkDescriptorSetLayout* set_layouts =
        stack_malloc(desc_count, VkDescriptorSetLayout);

    for (u32 i = 0; i < desc_count; i++)
    {
        set_layouts[i] = desc_layout;
    }
    VkDescriptorSetAllocateInfo alloc_info = { 0 };
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = desciptors->desc_pool;
    alloc_info.descriptorSetCount = desc_count;
    alloc_info.pSetLayouts = set_layouts;

    VK_ASSERT(vkAllocateDescriptorSets(device, &alloc_info, desciptors->desc_sets));

    update_descritors(region, device, desciptors, desc_count, texture, num_textures,
                      uniform_buffers);

    stack_end_scope(desc_stack);
}

void image_create(u32 width, u32 height, VkDevice device,
                  VkPhysicalDevice physical_device, VkFormat format,
                  VkImageTiling tiling, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags wanted_mem_props,
                  VkSampleCountFlagBits num_samples, u32 mip_map_lvl, VkImage* image,
                  VkDeviceMemory* image_mem)
{

    VkImageCreateInfo image_info = { 0 };
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

    memory_allocate(device, physical_device, mem_req, wanted_mem_props, image_mem);

    VK_ASSERT(vkBindImageMemory(device, *image, *image_mem, 0));
}

void image_view_create(VkDevice device, VkImage image,
                       VkImageViewType image_view_type, VkFormat image_format,
                       VkImageAspectFlags aspect_mask, u32 mip_map_lvl,
                       VkImageView* image_view)
{
    VkImageViewCreateInfo view_create_info = { 0 };
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.image = image;
    view_create_info.viewType = image_view_type;
    view_create_info.format = image_format;
    view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.subresourceRange.aspectMask = aspect_mask;
    view_create_info.subresourceRange.levelCount = mip_map_lvl;
    view_create_info.subresourceRange.layerCount = 1;

    VK_ASSERT(vkCreateImageView(device, &view_create_info, NULL, image_view));
}

void sampler_create(VkDevice device, Texture* textue)
{
    VkSamplerCreateInfo sampler_info = { 0 };
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

void buffer_image_copy(VkDevice device, VkCommandPool command_pool, u32 width,
                       u32 height, u32 mip_map_lvl, VkBuffer src_buffer,
                       VkImage dst_image, VkQueue graphics_queue,
                       VkDeviceSize size_bytes)
{
    VkCommandBuffer command_buff =
        command_buffer_begin(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkImageMemoryBarrier mem_barrier = { 0 };
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

    VkExtent3D image_extent = { 0 };
    image_extent.width = width;
    image_extent.height = height;
    image_extent.depth = 1;

    VkBufferImageCopy img_copy = { 0 };
    img_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_copy.imageSubresource.layerCount = 1;
    img_copy.imageExtent = image_extent;

    vkCmdCopyBufferToImage(command_buff, src_buffer, dst_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &img_copy);

    command_buffer_end(device, command_pool, command_buff, graphics_queue);
}

void bitmap_enable(VkDevice device, VkCommandPool command_pool,
                   VkQueue graphics_queue, VkImage image, const Texture* texture)
{
    VkCommandBuffer command_buff =
        command_buffer_begin(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkImageMemoryBarrier mem_barrier = { 0 };
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

        VkImageBlit blit = { 0 };
        blit.srcOffsets[0] = (VkOffset3D){ 0, 0, 0 };
        blit.srcOffsets[1] = (VkOffset3D){ w, h, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;

        if (w > 1) w /= 2;
        if (h > 1) h /= 2;

        blit.dstOffsets[0] = (VkOffset3D){ 0, 0, 0 };
        blit.dstOffsets[1] = (VkOffset3D){ w, h, 1 };
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

    command_buffer_end(device, command_pool, command_buff, graphics_queue);
}

void frame_buffer_create(VkDevice device, VkRenderPass render_pass,
                         VkExtent2D extent_2D, VkImageView img_view,
                         VkImageView depth_view, VkImageView color_view,
                         VkFramebuffer* framebuffer)
{
    VkImageView views[] = { color_view, depth_view, img_view };

    VkFramebufferCreateInfo framebuffer_info = { 0 };
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount = sy_SIZE(views);
    framebuffer_info.pAttachments = views;
    framebuffer_info.width = extent_2D.width;
    framebuffer_info.height = extent_2D.height;
    framebuffer_info.layers = 1;

    VK_ASSERT(vkCreateFramebuffer(device, &framebuffer_info, NULL, framebuffer));
}

u32 rand_rgb(u32 upper, u32 under)
{
    if (upper > 255) upper = 255;

    uint8_t r = (uint8_t)(rand() % (upper - under + 1) + under);

    uint8_t g = (uint8_t)(rand() % (upper - under + 1) + under);

    uint8_t b = (uint8_t)(rand() % (upper - under + 1) + under);

    return (u32)((u32)r | ((u32)g << 8) | ((u32)b << 16));
}

void texture_data_set(VkDevice device, VkPhysicalDevice physical_device, void* data,
                      VkCommandPool command_pool, VkQueue graphics_queue,
                      Texture* texture, VkDeviceSize size_bytes)
{
    Buffer staging_buffer = { 0 };
    staging_buffer.size_bytes = size_bytes;

    helper_buffer(device, physical_device, data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  &staging_buffer);

    buffer_image_copy(device, command_pool, texture->width, texture->height,
                      texture->mip_map_lvl, staging_buffer.buffer, texture->image,
                      graphics_queue, texture->size_bytes);

    buffer_destroy(device, staging_buffer);
}

i32 max_i(i32 f, i32 s)
{
    return (f > s) ? f : s;
}

void texture_path_create(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, VkFormat image_format, const char* tex_path,
                         Texture* texture)
{
    stack_begin_scope(text_stack);
    char* full_path = path_extend_d1(tex_path);
    i32 w, h, c;
    unsigned char* tex_buffer = stbi_load(full_path, &w, &h, &c, STBI_rgb_alpha);

    assert(tex_buffer);

    texture->size_bytes = (u32)w * h * 4;
    texture->width = (u32)w;
    texture->height = (u32)h;
    // Source: vulkan tutorial
    if (mip_map)
    {
        texture->mip_map_lvl = (u32)((log((f64)max_i(w, h)))) + 1;
    }
    else
    {
        texture->mip_map_lvl = 1;
    }

    image_create(texture->width, texture->height, device, physical_device,
                 image_format, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SAMPLE_COUNT_1_BIT,
                 texture->mip_map_lvl, &texture->image, &texture->img_memory);

    texture_data_set(device, physical_device, tex_buffer, command_pool,
                     graphics_queue, texture, texture->size_bytes);

    sampler_create(device, texture);

    image_view_create(device, texture->image, VK_IMAGE_VIEW_TYPE_2D, image_format,
                      VK_IMAGE_ASPECT_COLOR_BIT, texture->mip_map_lvl,
                      &texture->img_view);

    bitmap_enable(device, command_pool, graphics_queue, texture->image, texture);

    free(tex_buffer);
    stack_end_scope(text_stack);
}

u32 textures_path_create(VkDevice device, VkPhysicalDevice physical_device,
                         VkCommandPool command_pool, VkQueue graphics_queue,
                         b8 mip_map, u32 num_textures, const char** tex_paths,
                         Texture* textures)
{
    for (u32 i = 0; i < num_textures; i++)
    {
        texture_path_create(device, physical_device, command_pool, graphics_queue,
                            mip_map, VK_FORMAT_R8G8B8A8_SRGB, tex_paths[i],
                            textures + i);
    }
    return num_textures;
}

void texture_buffer_create(VkDevice device, VkPhysicalDevice physical_device,
                           VkCommandPool command_pool, VkQueue graphics_queue,
                           VkFormat image_format, Texture* texture,
                           unsigned char* tex_buffer)
{
    image_create(texture->width, texture->height, device, physical_device,
                 image_format, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SAMPLE_COUNT_1_BIT,
                 texture->mip_map_lvl, &texture->image, &texture->img_memory);

    texture_data_set(device, physical_device, tex_buffer, command_pool,
                     graphics_queue, texture, texture->size_bytes);

    sampler_create(device, texture);

    image_view_create(device, texture->image, VK_IMAGE_VIEW_TYPE_2D, image_format,
                      VK_IMAGE_ASPECT_COLOR_BIT, texture->mip_map_lvl,
                      &texture->img_view);

#if 1
    bitmap_enable(device, command_pool, graphics_queue, texture->image, texture);
#endif
}

void texture_create(VkDevice device, VkPhysicalDevice physical_device, u32 width,
                    u32 height, VkCommandPool command_pool, VkQueue graphics_queue,
                    Texture* texture)
{
    texture->width = width;
    texture->height = height;

    VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

    image_create(texture->width, texture->height, device, physical_device,
                 image_format, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SAMPLE_COUNT_1_BIT, 1,
                 &texture->image, &texture->img_memory);

    sampler_create(device, texture);

    image_view_create(device, texture->image, VK_IMAGE_VIEW_TYPE_2D, image_format,
                      VK_IMAGE_ASPECT_COLOR_BIT, 1, &texture->img_view);
}

void depth_image_create(VkDevice device, VkPhysicalDevice physical_device,
                        const VkExtent2D* extent_2D,
                        VkSampleCountFlagBits sample_count, Image* depth_image)
{
    VkFormat image_format = VK_FORMAT_D32_SFLOAT;

    image_create(extent_2D->width, extent_2D->height, device, physical_device,
                 image_format, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sample_count, 1,
                 &depth_image->image, &depth_image->img_memory);

    image_view_create(device, depth_image->image, VK_IMAGE_VIEW_TYPE_2D,
                      image_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1,
                      &depth_image->img_view);
}

#define sy_RGB(v) ((v) / 255.0f)

void render_pass_begin(VkCommandBuffer command_buffer, VkRenderPass render_pass,
                       VkFramebuffer framebuffer, const VkExtent2D* extent_2D)
{
    vkResetCommandBuffer(command_buffer, 0);

    VkCommandBufferBeginInfo buffer_begin_info = { 0 };
    buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_ASSERT(vkBeginCommandBuffer(command_buffer, &buffer_begin_info));

    VkClearValue clear_values[2] = { 0 };
    clear_values[0].color.float32[0] = sy_RGB(16.0f);
    clear_values[0].color.float32[1] = sy_RGB(26.0f);
    clear_values[0].color.float32[2] = sy_RGB(3.0f);
    clear_values[0].color.float32[3] = 1.0f;

    clear_values[1].depthStencil = (VkClearDepthStencilValue){ 1.0f, 0 };

    VkRenderPassBeginInfo render_pass_begin_info = { 0 };
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = render_pass;
    render_pass_begin_info.framebuffer = framebuffer;
    render_pass_begin_info.renderArea.extent.width = extent_2D->width;
    render_pass_begin_info.renderArea.extent.height = extent_2D->height;
    render_pass_begin_info.renderArea.offset = (VkOffset2D){ 0, 0 };
    render_pass_begin_info.clearValueCount = 2;
    render_pass_begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void render_pass_end(VkCommandBuffer command_buffer)
{
    vkCmdEndRenderPass(command_buffer);

    VK_ASSERT(vkEndCommandBuffer(command_buffer));
}

void graphics_pipline_bind(VkCommandBuffer command_buffer,
                           const Graphic_Pipeline* graphic_pipline,
                           u32 semaphore_idx)
{
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphic_pipline->pipeline);

    vkCmdBindDescriptorSets(
        command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphic_pipline->layout, 0,
        1, &graphic_pipline->descriptors.desc_sets[semaphore_idx], 0, NULL);
}

void push_constant(VkCommandBuffer command_buffer, VkPipelineLayout layout,
                   void* data, u32 size)
{
    vkCmdPushConstants(command_buffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, size,
                       data);
}

void draw(VkCommandBuffer command_buffer, u32 offset, u32 count)
{
    vkCmdDrawIndexed(command_buffer, count, 1, offset, 0, 0);
}

void vertex_index_buffer_bind(VkCommandBuffer command_buffer,
                              const Vertex_Buffer* vert_buffer,
                              const Index_Buffer* index_buffer)
{
    VkDeviceSize offset[] = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vert_buffer->buffer.buffer,
                           offset);
    vkCmdBindIndexBuffer(command_buffer, index_buffer->buffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);
}

void vertex_index_buffer1_bind(VkCommandBuffer command_buffer,
                               const Vertex_Index_Buffer* buffer)
{
    vertex_index_buffer_bind(command_buffer, &buffer->vert, &buffer->idx);
}

void data_buffer_copy(Buffer* buffer, void* data, size_t size_bytes)
{
    memcpy(buffer->transfer_data, data, size_bytes);
}

