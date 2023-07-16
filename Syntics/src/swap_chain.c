
u32 u32_clamp(u32 value, u32 min, u32 max)
{
    if (value > max)
        return max;
    else if (value < min)
        return min;
    return value;
}

VkSampleCountFlagBits max_usable_sample_count(VkPhysicalDevice physical_device)
{
    VkPhysicalDeviceProperties physical_device_props;
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_props);

    VkSampleCountFlags counts =
        physical_device_props.limits.framebufferColorSampleCounts &
        physical_device_props.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
    if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
    if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
    if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
    if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
    if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}

void swapchain_create(VkPhysicalDevice physical_device, VkDevice device,
                      VkSurfaceKHR surface, u32 width, u32 height,
                      Queue_Family_Indices indices, VkSwapchainKHR old_swap_chain,
                      Swap_Chain_Attrib* swap_chain)
{
    stack_begin_scope();

    VkSurfaceCapabilitiesKHR surface_cap;
    VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                                        &surface_cap));

    VkPresentModeKHR* present_modes = NULL;
    u32 present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                              &present_mode_count, NULL);

    VkPresentModeKHR present_mode_to_use = VK_PRESENT_MODE_FIFO_KHR;
    if (present_mode_count)
    {
        present_modes = stack_array(present_mode_count, VkPresentModeKHR);

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device, surface, &present_mode_count, present_modes);

        for (u32 i = 0; i < present_mode_count; i++)
        {
            if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                present_mode_to_use = present_modes[i];
                break;
            }
        }
    }

    VkSurfaceFormatKHR* surface_formats = NULL;
    u32 surface_format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                         &surface_format_count, NULL);

    if (surface_format_count)
    {
        surface_formats = stack_array(surface_format_count, VkSurfaceFormatKHR);

        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                             &surface_format_count, surface_formats);
    }
    else
    {
        SY_ERROR("Surface format count 0");
    }
    VkSurfaceFormatKHR surface_format_to_use = surface_formats[0];
    for (u32 i = 0; i < surface_format_count; i++)
    {
        if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surface_format_to_use = surface_formats[i];
            break;
        }
    }

    VkExtent2D extent_2D = surface_cap.currentExtent;
    if (surface_cap.currentExtent.width == 0xFFFFFFFF)
    {
        extent_2D.width = u32_clamp(width, surface_cap.minImageExtent.width,
                                    surface_cap.maxImageExtent.width);

        extent_2D.height = u32_clamp(height, surface_cap.minImageExtent.height,
                                     surface_cap.maxImageExtent.height);
    }
    uint32_t min_image_count = surface_cap.minImageCount + 1;
    if (min_image_count > surface_cap.maxImageCount && surface_cap.maxImageCount > 0)
    {
        min_image_count = surface_cap.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swap_info = { 0 };
    swap_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_info.surface = surface;
    swap_info.minImageCount = min_image_count;
    swap_info.imageFormat = surface_format_to_use.format;
    swap_info.imageColorSpace = surface_format_to_use.colorSpace;
    swap_info.imageExtent = extent_2D;
    swap_info.imageArrayLayers = 1;
    swap_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swap_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap_info.queueFamilyIndexCount = 0;
    swap_info.pQueueFamilyIndices = NULL;
    swap_info.preTransform = surface_cap.currentTransform;
    swap_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_info.presentMode = present_mode_to_use;
    swap_info.clipped = VK_FALSE;
    swap_info.oldSwapchain = old_swap_chain;
    if (indices.num_index_fam > 1)
    {
        swap_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_info.queueFamilyIndexCount = indices.num_index_fam;
        swap_info.pQueueFamilyIndices = indices.indices;
    }

    swap_chain->swap_chain = VK_NULL_HANDLE;
    swap_chain->color_format = surface_format_to_use.format;
    swap_chain->extent_2D = extent_2D;
    // swap_chain->sample_count = max_usable_sample_count(physical_device);
    swap_chain->sample_count = VK_SAMPLE_COUNT_2_BIT;

    VK_ASSERT(
        vkCreateSwapchainKHR(device, &swap_info, NULL, &swap_chain->swap_chain));

    stack_end_scope();
}

void render_pass_create(VkDevice device, VkFormat color_format,
                        VkSampleCountFlagBits sample_count,
                        VkRenderPass* render_pass)
{
    // If the attachment uses a color format, then loadOp and storeOp are used,
    // and stencilLoadOp and stencilStoreOp are ignored.
    // If the format has depth and/or stencil components, loadOp and storeOp
    // apply only to the depth data, while stencilLoadOp and stencilStoreOp
    // define how the stencil data is handled.
    // If a set of attachments alias each other, then all except the first to be
    // used in the render pass must use an initialLayout of
    // VK_IMAGE_LAYOUT_UNDEFINED. -Vulkan Specification
    //
    //
    VkAttachmentDescription attachment_descs[3] = { 0 };

    VkAttachmentDescription color_attach_desc = { 0 };
    color_attach_desc.format = color_format;
    color_attach_desc.samples = sample_count;
    color_attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attach_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachment_descs[0] = color_attach_desc;

    VkAttachmentDescription depth_attach_desc = { 0 };
    depth_attach_desc.format = VK_FORMAT_D32_SFLOAT;
    depth_attach_desc.samples = sample_count;
    depth_attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachment_descs[1] = depth_attach_desc;

    // Need to resolve multisampled image to normal one for presenting.
    VkAttachmentDescription resolve_image_desc = { 0 };
    resolve_image_desc.format = color_format;
    resolve_image_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    resolve_image_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolve_image_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    resolve_image_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolve_image_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    resolve_image_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    resolve_image_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachment_descs[2] = resolve_image_desc;

    VkAttachmentReference color_attach_ref = { 0 };
    color_attach_ref.attachment = 0;
    color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attach_ref = { 0 };
    depth_attach_ref.attachment = 1;
    depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference resolve_attach_ref = { 0 };
    resolve_attach_ref.attachment = 2;
    resolve_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_desc = { 0 };
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.colorAttachmentCount = 1;
    subpass_desc.pColorAttachments = &color_attach_ref;
    subpass_desc.pDepthStencilAttachment = &depth_attach_ref;
    subpass_desc.pResolveAttachments = &resolve_attach_ref;

    VkSubpassDependency dependency = { 0 };
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = { 0 };
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = sy_SIZE(attachment_descs);
    render_pass_info.pAttachments = attachment_descs;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass_desc;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VK_ASSERT(vkCreateRenderPass(device, &render_pass_info, NULL, render_pass));
}

void swapchain_images_get(Region_Alloc* region, VkDevice device,
                          Swap_Chain_Attrib* swap_chain)
{
    vkGetSwapchainImagesKHR(device, swap_chain->swap_chain, &swap_chain->num_images,
                            NULL);

    if (!swap_chain->images)
        swap_chain->images =
            region_array(region, swap_chain->num_images, VkImage, PERM_ARRAY);

    vkGetSwapchainImagesKHR(device, swap_chain->swap_chain, &swap_chain->num_images,
                            swap_chain->images);

    assert(array_capacity(swap_chain->images) == swap_chain->num_images);
}

void graphics_pipeline_create(VkDevice device, VkRenderPass render_pass,
                              VkSampleCountFlagBits sample_count,
                              const char* vert_path, const char* frag_path,
                              u32 width, u32 height, u32 num_textures,
                              const VkRect2D* sciss,
                              Graphic_Pipeline* graphic_pipline)
{
    stack_begin_scope();

    char* full_vert_path = extend_path_d1(vert_path);
    char* full_frag_path = extend_path_d1(frag_path);

    File_Attrib vert_file;
    read_file(&vert_file, get_stack(), full_vert_path, "rb");
    File_Attrib frag_file;
    read_file(&frag_file, get_stack(), full_frag_path, "rb");

    VkShaderModuleCreateInfo vertex_info = { 0 };
    vertex_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertex_info.codeSize = vert_file.size;
    vertex_info.pCode = (const u32*)vert_file.buffer;

    VkShaderModuleCreateInfo frag_info = { 0 };
    frag_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    frag_info.codeSize = frag_file.size;
    frag_info.pCode = (const u32*)frag_file.buffer;

    VkShaderModule vertex_module = VK_NULL_HANDLE;
    VkShaderModule frag_module = VK_NULL_HANDLE;

    VK_ASSERT(vkCreateShaderModule(device, &vertex_info, NULL, &vertex_module));
    VK_ASSERT(vkCreateShaderModule(device, &frag_info, NULL, &frag_module));

    VkPipelineShaderStageCreateInfo shader_stages[2] = { 0 };

    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = vertex_module;
    shader_stages[0].pName = "main";

    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = frag_module;
    shader_stages[1].pName = "main";

    VkGraphicsPipelineCreateInfo PIPELINE_CREATE_INFO = { 0 };

    PIPELINE_CREATE_INFO.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PIPELINE_CREATE_INFO.renderPass = render_pass;
    PIPELINE_CREATE_INFO.stageCount = sy_SIZE(shader_stages);
    PIPELINE_CREATE_INFO.pStages = shader_stages;

    VkVertexInputBindingDescription binding_desc = { 0 };
    binding_desc.binding = 0;
    binding_desc.stride = sizeof(Vertex);
    binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vert_attrib_descs[5] = { 0 };

    vert_attrib_descs[0].location = 0;
    vert_attrib_descs[0].binding = 0;
    vert_attrib_descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vert_attrib_descs[0].offset = offsetof(Vertex, pos);

    vert_attrib_descs[1].location = 1;
    vert_attrib_descs[1].binding = 0;
    vert_attrib_descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vert_attrib_descs[1].offset = offsetof(Vertex, normal);

    vert_attrib_descs[2].location = 2;
    vert_attrib_descs[2].binding = 0;
    vert_attrib_descs[2].format = VK_FORMAT_R32G32_SFLOAT;
    vert_attrib_descs[2].offset = offsetof(Vertex, tex_coords);

    vert_attrib_descs[3].location = 3;
    vert_attrib_descs[3].binding = 0;
    vert_attrib_descs[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vert_attrib_descs[3].offset = offsetof(Vertex, color);

    vert_attrib_descs[4].location = 4;
    vert_attrib_descs[4].binding = 0;
    vert_attrib_descs[4].format = VK_FORMAT_R32_SFLOAT;
    vert_attrib_descs[4].offset = offsetof(Vertex, tex_index);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = { 0 };
    vertex_input_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = sy_SIZE(vert_attrib_descs);
    vertex_input_info.pVertexAttributeDescriptions = vert_attrib_descs;

    PIPELINE_CREATE_INFO.pVertexInputState = &vertex_input_info;

    if (graphic_pipline->topology > VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
    {
        graphic_pipline->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
    VkPipelineInputAssemblyStateCreateInfo assembly_create_info = { 0 };
    assembly_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assembly_create_info.topology = graphic_pipline->topology;

    PIPELINE_CREATE_INFO.pInputAssemblyState = &assembly_create_info;

    VkViewport view_port = { 0 };
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = (f32)width;
    view_port.height = (f32)height;
    view_port.minDepth = 0.0f;
    view_port.maxDepth = 1.0f;

    VkRect2D scissor = { 0 };
    if (sciss == NULL)
    {
        scissor.extent.width = width;
        scissor.extent.height = height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
    }
    else
    {
        scissor = *sciss;
    }

    VkPipelineViewportStateCreateInfo view_port_info = { 0 };
    view_port_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    view_port_info.viewportCount = 1;
    view_port_info.pViewports = &view_port;
    view_port_info.scissorCount = 1;
    view_port_info.pScissors = &scissor;

    PIPELINE_CREATE_INFO.pViewportState = &view_port_info;

    VkPipelineRasterizationStateCreateInfo rasterizer_info = { 0 };
    rasterizer_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_info.polygonMode = graphic_pipline->poly_mode;
    if (graphic_pipline->line_width > 1.0f)
    {
        rasterizer_info.lineWidth = graphic_pipline->line_width;
    }
    else
    {
        rasterizer_info.lineWidth = 1.0f;
    }
    rasterizer_info.cullMode = graphic_pipline->cull_mode;
    rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_info.depthBiasEnable = VK_TRUE; // Z fighting
    rasterizer_info.depthBiasConstantFactor = 1.0f;
    rasterizer_info.depthBiasClamp = 0.0f;
    rasterizer_info.depthBiasSlopeFactor = 1.0f;
    /*
     * By adjusting the depth bias values, you can push the depth values of one or
     * more primitives slightly closer to the camera to avoid depth fighting. The
     * depthBiasConstantFactor and depthBiasSlopeFactor values control the magnitude
     * of the depth bias, while the depthBiasClamp value ensures that the depth bias
     * is not too large. - Chat.gpt
     * */
    PIPELINE_CREATE_INFO.pRasterizationState = &rasterizer_info;

    VkPipelineColorBlendAttachmentState color_blend_attach = { 0 };
#if 0
    color_blend_attach.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attach.blendEnable = VK_TRUE;
#endif
#if 1
    color_blend_attach.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attach.blendEnable = VK_TRUE;
    color_blend_attach.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attach.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attach.alphaBlendOp = VK_BLEND_OP_ADD;
#endif

    VkPipelineColorBlendStateCreateInfo color_blend_info = { 0 };
    color_blend_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.logicOpEnable = VK_FALSE;
    color_blend_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_info.attachmentCount = 1;
    color_blend_info.pAttachments = &color_blend_attach;

    PIPELINE_CREATE_INFO.pColorBlendState = &color_blend_info;

#if 1
    VkDescriptorSetLayoutBinding layout_binding[2] = { 0 };

    layout_binding[0].binding = 0;
    layout_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding[0].descriptorCount = 1;
    layout_binding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    layout_binding[1].binding = 1;
    layout_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_binding[1].descriptorCount = num_textures;
    layout_binding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
#else
    VkDescriptorSetLayoutBinding layout_binding[1] = { 0 };
    layout_binding[0].binding = 0;
    layout_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_binding[0].descriptorCount = num_textures;
    layout_binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
#endif

    ///    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT
    ///    set_layout_binding_flags{}; set_layout_binding_flags.sType =
    ///        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    ///    set_layout_binding_flags.bindingCount                  = 2;
    ///    VkDescriptorBindingFlagsEXT descriptor_binding_flags[] = {
    ///        0, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
    ///    };
    ///    set_layout_binding_flags.pBindingFlags = descriptor_binding_flags;

    VkDescriptorSetLayoutCreateInfo set_layout_info = { 0 };
    set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_layout_info.bindingCount = sy_SIZE(layout_binding);
    set_layout_info.pBindings = layout_binding;

    VK_ASSERT(vkCreateDescriptorSetLayout(device, &set_layout_info, NULL,
                                          &graphic_pipline->set_layout));

    VkPushConstantRange p_c_range = { 0 };
    p_c_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    p_c_range.offset = 0;
    p_c_range.size = sizeof(VP);

    VkPipelineLayoutCreateInfo layout_info = { 0 };
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = &graphic_pipline->set_layout;
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = &p_c_range;

    VK_ASSERT(vkCreatePipelineLayout(device, &layout_info, NULL,
                                     &graphic_pipline->layout));

    PIPELINE_CREATE_INFO.layout = graphic_pipline->layout;

    // NOTE: disable write to get transparancy in 2D
    // vkCmdSetDepthWriteEnable

    VkPipelineDepthStencilStateCreateInfo depth_info = { 0 };
    depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_info.depthTestEnable = VK_TRUE;
    depth_info.depthWriteEnable = VK_TRUE;
    // This helps as well with transparancy in 2D
    depth_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    PIPELINE_CREATE_INFO.pDepthStencilState = &depth_info;

    VkPipelineMultisampleStateCreateInfo multisampling = { 0 };
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = sample_count;

    PIPELINE_CREATE_INFO.pMultisampleState = &multisampling;

    u32 deduction = 0;
    for (u32 i = 0; i < graphic_pipline->dynamic; i++)
    {
        if (graphic_pipline->dynamic_states[i] == VK_DYNAMIC_STATE_SCISSOR ||
            VK_DYNAMIC_STATE_VIEWPORT)
        {
            deduction++;
        }
    }
    graphic_pipline->dynamic -= deduction <= 2 ? deduction : 2;

    const u32 dynamic_states_count = 2 + graphic_pipline->dynamic;
    VkDynamicState* dyn_states = stack_calloc(dynamic_states_count, VkDynamicState);
    dyn_states[0] = VK_DYNAMIC_STATE_SCISSOR;
    dyn_states[1] = VK_DYNAMIC_STATE_VIEWPORT;
    for (u32 i = 2; i < dynamic_states_count; i++)
    {
        dyn_states[i] = graphic_pipline->dynamic_states[i];
    }
    VkPipelineDynamicStateCreateInfo dyn_info = { 0 };
    dyn_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn_info.dynamicStateCount = dynamic_states_count;
    dyn_info.pDynamicStates = dyn_states;

    PIPELINE_CREATE_INFO.pDynamicState = &dyn_info;

    PIPELINE_CREATE_INFO.subpass = 0;

    VK_ASSERT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                        &PIPELINE_CREATE_INFO, NULL,
                                        &graphic_pipline->pipeline));

    vkDestroyShaderModule(device, vertex_module, NULL);
    vkDestroyShaderModule(device, frag_module, NULL);

    stack_end_scope();
}

void uniforms_descriptors_init(Region_Alloc* region, VkDevice device,
                               VkPhysicalDevice physical_device,
                               Uniform_Buffer** uniform_buffers,
                               Descriptors* descriptors,
                               VkDescriptorSetLayout set_layout, u32 num_semaphores,
                               const Texture* textures, u32 num_textures)
{
    *uniform_buffers = region_mallocP(region, num_semaphores, Uniform_Buffer);
    descriptors->desc_sets = region_mallocP(region, num_semaphores, VkDescriptorSet);

    for (u32 i = 0; i < num_semaphores; i++)
    {
        (*uniform_buffers)[i].buffer.size_bytes = (u32)sizeof(VP);

        create_uniform_buffer(device, physical_device, (*uniform_buffers) + i);
    }
    create_descriptors(region, device, descriptors, num_semaphores, set_layout,
                       textures, num_textures, *uniform_buffers);
}

void graphics_pipeline_init(Region_Alloc* region, VkDevice device,
                            VkPhysicalDevice physical_device, u32 num_semaphores,
                            const Texture* textures, u32 num_textures,
                            Graphic_Pipeline* gp)
{
    uniforms_descriptors_init(region, device, physical_device, &gp->uniform_buffers,
                              &gp->descriptors, gp->set_layout, num_semaphores,
                              textures, num_textures);
}

void graphics_pipeline_create_deluxe(Region_Alloc* region, VkDevice device,
                                     VkPhysicalDevice phy_device, u32 num_semaphores,
                                     const char* vert_path, const char* frag_path,
                                     const Swap_Chain_Attrib* swap_chain,
                                     const Texture* textures, u32 num_textures,
                                     Graphic_Pipeline* graphic_pipline)
{
    graphics_pipeline_create(
        device, swap_chain->render_pass, swap_chain->sample_count, vert_path,
        frag_path, swap_chain->extent_2D.width, swap_chain->extent_2D.height,
        num_textures, NULL, graphic_pipline);

    init_graphics_pipeline(region, device, phy_device, num_semaphores, textures,
                           num_textures, graphic_pipline);
}

void multisample_enable(const Swap_Chain_Attrib* swap_chain, VkDevice device,
                        VkPhysicalDevice physical_device, Image* color_image)
{
    create_image(swap_chain->extent_2D.width, swap_chain->extent_2D.height, device,
                 physical_device, swap_chain->color_format, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, swap_chain->sample_count, 1,
                 &color_image->image, &color_image->img_memory);

    image_view_create(device, color_image->image, VK_IMAGE_VIEW_TYPE_2D,
                      swap_chain->color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1,
                      &color_image->img_view);
}

void graphic_pipline_ap_recreate(const Application_State* app_state,
                                 const char* vert_file, const char* frag_file,
                                 Graphic_Pipeline* graphic_pipline, u32 num_textures,
                                 const VkRect2D* scissor)
{
    vkDeviceWaitIdle(app_state->device);

    vkDestroyPipelineLayout(app_state->device, graphic_pipline->layout, NULL);
    vkDestroyPipeline(app_state->device, graphic_pipline->pipeline, NULL);
    vkDestroyDescriptorSetLayout(app_state->device, graphic_pipline->set_layout,
                                 NULL);

    graphics_pipeline_create(app_state->device, app_state->swap_chain.render_pass,
                             app_state->swap_chain.sample_count, vert_file,
                             frag_file, app_state->swap_chain.extent_2D.width,
                             app_state->swap_chain.extent_2D.height, num_textures,
                             scissor, graphic_pipline);
}

void graphic_pipline_sw_recreate(VkDevice device,
                                 const Swap_Chain_Attrib* swap_chain,
                                 const char* vert_file, const char* frag_file,
                                 Graphic_Pipeline* graphic_pipline, u32 num_textures,
                                 const VkRect2D* scissor)
{
    vkDeviceWaitIdle(device);

    vkDestroyPipelineLayout(device, graphic_pipline->layout, NULL);
    vkDestroyPipeline(device, graphic_pipline->pipeline, NULL);
    vkDestroyDescriptorSetLayout(device, graphic_pipline->set_layout, NULL);

    graphics_pipeline_create(
        device, swap_chain->render_pass, swap_chain->sample_count, vert_file,
        frag_file, swap_chain->extent_2D.width, swap_chain->extent_2D.height,
        num_textures, scissor, graphic_pipline);
}

void swapchain_recreate(Region_Alloc* region, Application_State* app_state,
                        u32 width, u32 height)
{
    vkDeviceWaitIdle(app_state->device);

    for (u32 i = 0; i < app_state->swap_chain.num_images; i++)
    {
        vkDestroyFramebuffer(app_state->device,
                             app_state->swap_chain.framebuffers[i], NULL);
        vkDestroyImageView(app_state->device, app_state->swap_chain.img_views[i],
                           NULL);
    }
    VkSwapchainKHR old_swap_chain = app_state->swap_chain.swap_chain;
   swapchain_create(app_state->phy_device, app_state->device, app_state->surface,
                     width, height, app_state->q_indices, old_swap_chain,
                     &app_state->swap_chain);

    vkDestroySwapchainKHR(app_state->device, old_swap_chain, NULL);

    vkDestroyRenderPass(app_state->device, app_state->swap_chain.render_pass, NULL);

    image_destroy(app_state->device, app_state->depth_img);
    image_destroy(app_state->device, app_state->color_img);

    multisample_enable(&app_state->swap_chain, app_state->device,
                       app_state->phy_device, &app_state->color_img);

    depth_image_create(app_state->device, app_state->phy_device,
                       &app_state->swap_chain.extent_2D,
                       app_state->swap_chain.sample_count, &app_state->depth_img);

    swapchain_images_get(region, app_state->device, &app_state->swap_chain);

    render_pass_create(app_state->device, app_state->swap_chain.color_format,
                       app_state->swap_chain.sample_count,
                       &app_state->swap_chain.render_pass);

    ASSERT(array_capacity(app_state->swap_chain.img_views) ==
               app_state->swap_chain.num_images,
           "");

    ASSERT(array_capacity(app_state->swap_chain.framebuffers) ==
               app_state->swap_chain.num_images,
           "");

    for (u32 i = 0; i < app_state->swap_chain.num_images; i++)
    {
        image_view_create(app_state->device, app_state->swap_chain.images[i],
                          VK_IMAGE_VIEW_TYPE_2D, app_state->swap_chain.color_format,
                          VK_IMAGE_ASPECT_COLOR_BIT, 1,
                          &app_state->swap_chain.img_views[i]);

        frame_buffer_create(
            app_state->device, app_state->swap_chain.render_pass,
            app_state->swap_chain.extent_2D, app_state->swap_chain.img_views[i],
            app_state->depth_img.img_view, app_state->color_img.img_view,
            &app_state->swap_chain.framebuffers[i]);
    }
}

void graphic_pipeline_destroy(VkDevice device, u32 num_semaphores,
                              Graphic_Pipeline* gp)
{

    vkDestroyPipelineLayout(device, gp->layout, NULL);
    vkDestroyPipeline(device, gp->pipeline, NULL);
    vkDestroyDescriptorSetLayout(device, gp->set_layout, NULL);
    vkDestroyDescriptorPool(device, gp->descriptors.desc_pool, NULL);
    for (u32 i = 0; i < num_semaphores; i++)
    {
        buffer_destroy(device, gp->uniform_buffers[i].buffer);
    }
}

