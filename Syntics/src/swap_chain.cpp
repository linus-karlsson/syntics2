#include "swap_chain.h"
#include "buffers.h"
#include "region_alloc.h"
#include "file_reading.h"

namespace synt {

static uint32 clamp_u32(uint32 value, uint32 min, uint32 max)
{
    if (value > max)
        return max;
    else if (value < min)
        return min;
    return value;
}

void create_swapchain(Region_Alloc* region, VkPhysicalDevice physical_device,
                      VkDevice device, VkSurfaceKHR surface, uint32 width,
                      uint32 height, Queue_Family_Indices indices,
                      Swap_Chain_attrib* swap_chain)
{

    VkSurfaceCapabilitiesKHR surface_cap;
    VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,
                                                        surface, &surface_cap));

    Temp_Alloc<VkPresentModeKHR> present_modes;
    uint32 present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                              &present_mode_count, NULL);
    if (present_mode_count)
    {
        present_modes.init(region, present_mode_count);

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device, surface, &present_mode_count, present_modes.data);
    }

    Temp_Alloc<VkSurfaceFormatKHR> surface_formats;
    uint32 surface_format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                         &surface_format_count, NULL);

    if (surface_format_count)
    {
        surface_formats.init(region, surface_format_count);

        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                             &surface_format_count,
                                             surface_formats.data);
    }

    VkPresentModeKHR present_mode_to_use = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32 i = 0; i < present_mode_count; i++)
    {
        if (present_modes.data[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode_to_use = present_modes.data[i];
            break;
        }
    }
    VkSurfaceFormatKHR surface_format_to_use = surface_formats.data[0];
    for (uint32 i = 0; i < surface_format_count; i++)
    {
        if (surface_formats.data[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            surface_formats.data[i].colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surface_format_to_use = surface_formats.data[i];
            break;
        }
    }

    VkExtent2D extent_2D = surface_cap.currentExtent;
    if (surface_cap.currentExtent.width == 0xFFFFFFFF)
    {
        extent_2D.width = clamp_u32(width, surface_cap.minImageExtent.width,
                                    surface_cap.maxImageExtent.width);

        extent_2D.height = clamp_u32(height, surface_cap.minImageExtent.height,
                                     surface_cap.maxImageExtent.height);
    }

    uint32_t min_image_count = surface_cap.minImageCount + 1;
    if (min_image_count > surface_cap.maxImageCount &&
        surface_cap.maxImageCount > 0)
    {
        min_image_count = surface_cap.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swap_info = {};
    swap_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_info.surface          = surface;
    swap_info.minImageCount    = min_image_count;
    swap_info.imageFormat      = surface_format_to_use.format;
    swap_info.imageColorSpace  = surface_format_to_use.colorSpace;
    swap_info.imageExtent      = extent_2D;
    swap_info.imageArrayLayers = 1;
    swap_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swap_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap_info.queueFamilyIndexCount = 0;
    swap_info.pQueueFamilyIndices   = NULL;
    swap_info.preTransform          = surface_cap.currentTransform;
    swap_info.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_info.presentMode           = present_mode_to_use;
    swap_info.clipped               = VK_FALSE;
    swap_info.oldSwapchain          = VK_NULL_HANDLE;
    if (indices.num_index_fam > 1)
    {
        swap_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swap_info.queueFamilyIndexCount = indices.num_index_fam;
        swap_info.pQueueFamilyIndices   = indices.indices;
    }

    swap_chain->swap_chain   = VK_NULL_HANDLE;
    swap_chain->color_format = surface_format_to_use.format;
    swap_chain->extent_2D    = extent_2D;

    VK_ASSERT(vkCreateSwapchainKHR(device, &swap_info, NULL,
                                   &swap_chain->swap_chain));
}

void create_render_pass(VkDevice device, VkFormat color_format,
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
    VkAttachmentDescription attachment_descs[2] = {};

    VkAttachmentDescription color_attach_desc = {};
    color_attach_desc.format                  = color_format;
    color_attach_desc.samples                 = VK_SAMPLE_COUNT_1_BIT;
    color_attach_desc.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attach_desc.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    color_attach_desc.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attach_desc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attach_desc.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachment_descs[0] = color_attach_desc;

    VkAttachmentDescription depth_attach_desc = {};
    depth_attach_desc.format                  = VK_FORMAT_D32_SFLOAT;
    depth_attach_desc.samples                 = VK_SAMPLE_COUNT_1_BIT;
    depth_attach_desc.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attach_desc.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attach_desc.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attach_desc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attach_desc.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachment_descs[1] = depth_attach_desc;

    VkAttachmentReference color_attach_ref = {};
    color_attach_ref.attachment            = 0;
    color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attach_ref = {};
    depth_attach_ref.attachment            = 1;
    depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_desc    = {};
    subpass_desc.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.colorAttachmentCount    = 1;
    subpass_desc.pColorAttachments       = &color_attach_ref;
    subpass_desc.pDepthStencilAttachment = &depth_attach_ref;

    VkSubpassDependency subpass_dependency = {};
    subpass_dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass          = 0;
    subpass_dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = sy_size(attachment_descs);
    render_pass_info.pAttachments    = attachment_descs;
    render_pass_info.subpassCount    = 1;
    render_pass_info.pSubpasses      = &subpass_desc;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies   = &subpass_dependency;

    VK_ASSERT(vkCreateRenderPass(device, &render_pass_info, NULL, render_pass));
}

void get_swapchain_images(Region_Alloc* region, VkDevice device,
                          Swap_Chain_attrib* swap_chain)
{
    vkGetSwapchainImagesKHR(device, swap_chain->swap_chain,
                            &swap_chain->num_images, NULL);

    if (!swap_chain->images)
        swap_chain->images = dyn_array((*region), swap_chain->num_images,
                                       VkImage, synt::PERM_ARRAY);

    vkGetSwapchainImagesKHR(device, swap_chain->swap_chain,
                            &swap_chain->num_images, swap_chain->images);

    assert(synt::capacity_arr(swap_chain->images) == swap_chain->num_images);
}

void create_image_view(VkDevice device, VkImage image,
                       VkImageViewType image_view_type, VkFormat image_format,
                       VkImageAspectFlags aspect_mask, VkImageView* image_view)
{
    VkImageViewCreateInfo view_create_info = {};
    view_create_info.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.image        = image;
    view_create_info.viewType     = image_view_type;
    view_create_info.format       = image_format;
    view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.subresourceRange.aspectMask = aspect_mask;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.layerCount = 1;

    VK_ASSERT(vkCreateImageView(device, &view_create_info, NULL, image_view));
}

void create_frame_buffer(VkDevice device, VkRenderPass render_pass,
                         VkExtent2D extent_2D, VkImageView img_view,
                         VkImageView depth_view, VkFramebuffer* framebuffer)
{
    VkImageView views[] = { img_view, depth_view };

    VkFramebufferCreateInfo framebuffer_info = {};
    framebuffer_info.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount = sy_size(views);
    framebuffer_info.pAttachments    = views;
    framebuffer_info.width           = extent_2D.width;
    framebuffer_info.height          = extent_2D.height;
    framebuffer_info.layers          = 1;

    VK_ASSERT(
        vkCreateFramebuffer(device, &framebuffer_info, NULL, framebuffer));
}

void create_graphics_pipeline(Region_Alloc* region, VkDevice device,
                              VkFormat format, VkRenderPass render_pass,
                              const char* vert_path, const char* frag_path,
                              uint32 width, uint32 height,
                              Graphic_Pipline* graphic_pipline)
{

    File_Attrib vert_file = read_file(region, vert_path, "rb");
    File_Attrib frag_file = read_file(region, frag_path, "rb");

    VkShaderModuleCreateInfo vertex_info = {};
    vertex_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertex_info.codeSize = vert_file.size;
    vertex_info.pCode    = (const uint32*)vert_file.buffer;

    VkShaderModuleCreateInfo frag_info = {};
    frag_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    frag_info.codeSize = frag_file.size;
    frag_info.pCode    = (const uint32*)frag_file.buffer;

    VkShaderModule vertex_module = VK_NULL_HANDLE;
    VkShaderModule frag_module   = VK_NULL_HANDLE;

    VK_ASSERT(vkCreateShaderModule(device, &vertex_info, NULL, &vertex_module));
    VK_ASSERT(vkCreateShaderModule(device, &frag_info, NULL, &frag_module));

    // Empty region stack
    region_pop((*region), frag_file.size, char, TEMP_MALLOC);
    region_pop((*region), vert_file.size, char, TEMP_MALLOC);

    VkPipelineShaderStageCreateInfo shader_stages[2] = {};

    shader_stages[0].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = vertex_module;
    shader_stages[0].pName  = "main";

    shader_stages[1].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = frag_module;
    shader_stages[1].pName  = "main";

    VkGraphicsPipelineCreateInfo PIPELINE_CREATE_INFO = {};
    PIPELINE_CREATE_INFO.sType =
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PIPELINE_CREATE_INFO.renderPass = render_pass;
    PIPELINE_CREATE_INFO.stageCount = 2;
    PIPELINE_CREATE_INFO.pStages    = shader_stages;

    VkVertexInputBindingDescription binding_desc = {};
    binding_desc.binding                         = 0;
    binding_desc.stride                          = sizeof(Vertex);
    binding_desc.inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vert_attrib_descs[4] = {};

    vert_attrib_descs[0].location = 0;
    vert_attrib_descs[0].binding  = 0;
    vert_attrib_descs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    vert_attrib_descs[0].offset   = offsetof(Vertex, pos);

    vert_attrib_descs[1].location = 1;
    vert_attrib_descs[1].binding  = 0;
    vert_attrib_descs[1].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
    vert_attrib_descs[1].offset   = offsetof(Vertex, color);

    vert_attrib_descs[2].location = 2;
    vert_attrib_descs[2].binding  = 0;
    vert_attrib_descs[2].format   = VK_FORMAT_R32G32_SFLOAT;
    vert_attrib_descs[2].offset   = offsetof(Vertex, tex_coords);

    vert_attrib_descs[3].location = 3;
    vert_attrib_descs[3].binding  = 0;
    vert_attrib_descs[3].format   = VK_FORMAT_R32_SFLOAT;
    vert_attrib_descs[3].offset   = offsetof(Vertex, tex_index);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions    = &binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount =
        sy_size(vert_attrib_descs);
    vertex_input_info.pVertexAttributeDescriptions = vert_attrib_descs;

    PIPELINE_CREATE_INFO.pVertexInputState = &vertex_input_info;

    VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
    assembly_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    PIPELINE_CREATE_INFO.pInputAssemblyState = &assembly_create_info;

    VkViewport view_port = {};
    view_port.width      = width;
    view_port.height     = height;
    view_port.minDepth   = 0.0f;
    view_port.maxDepth   = 1.0f;

    VkRect2D scissor      = {};
    scissor.extent.width  = width;
    scissor.extent.height = height;
    scissor.offset.x      = 0;
    scissor.offset.y      = 0;

    VkPipelineViewportStateCreateInfo view_port_info = {};
    view_port_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    view_port_info.viewportCount = 1;
    view_port_info.pViewports    = &view_port;
    view_port_info.scissorCount  = 1;
    view_port_info.pScissors     = &scissor;

    PIPELINE_CREATE_INFO.pViewportState = &view_port_info;

    VkPipelineRasterizationStateCreateInfo rasterizer_info = {};
    rasterizer_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_info.cullMode    = VK_CULL_MODE_BACK_BIT;
    rasterizer_info.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_info.lineWidth   = 1.0f;

    PIPELINE_CREATE_INFO.pRasterizationState = &rasterizer_info;

    // This might be wrong.
    VkPipelineColorBlendAttachmentState color_blend_attach = {};
    color_blend_attach.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_info = {};
    color_blend_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.logicOpEnable   = VK_FALSE;
    color_blend_info.logicOp         = VK_LOGIC_OP_COPY;
    color_blend_info.attachmentCount = 1;
    color_blend_info.pAttachments    = &color_blend_attach;

    PIPELINE_CREATE_INFO.pColorBlendState = &color_blend_info;

    VkDescriptorSetLayoutBinding layout_binding[2] = {};

    layout_binding[0].binding         = 0;
    layout_binding[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding[0].descriptorCount = 1;
    layout_binding[0].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

    layout_binding[1].binding = 1;
    layout_binding[1].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_binding[1].descriptorCount = 2;
    layout_binding[1].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

    ///    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT
    ///    set_layout_binding_flags{}; set_layout_binding_flags.sType =
    ///        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    ///    set_layout_binding_flags.bindingCount                  = 2;
    ///    VkDescriptorBindingFlagsEXT descriptor_binding_flags[] = {
    ///        0, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
    ///    };
    ///    set_layout_binding_flags.pBindingFlags = descriptor_binding_flags;

    VkDescriptorSetLayoutCreateInfo set_layout_info = {};
    set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_layout_info.bindingCount = sy_size(layout_binding);
    set_layout_info.pBindings    = layout_binding;

    VK_ASSERT(vkCreateDescriptorSetLayout(device, &set_layout_info, NULL,
                                          &graphic_pipline->set_layout));

    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts    = &graphic_pipline->set_layout;

    VK_ASSERT(vkCreatePipelineLayout(device, &layout_info, NULL,
                                     &graphic_pipline->layout));

    PIPELINE_CREATE_INFO.layout = graphic_pipline->layout;

    VkPipelineDepthStencilStateCreateInfo depth_info = {};
    depth_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_info.depthTestEnable  = VK_TRUE;
    depth_info.depthWriteEnable = VK_TRUE;
    depth_info.depthCompareOp   = VK_COMPARE_OP_LESS;

    PIPELINE_CREATE_INFO.pMultisampleState  = VK_NULL_HANDLE;
    PIPELINE_CREATE_INFO.pDepthStencilState = &depth_info;
    PIPELINE_CREATE_INFO.pDynamicState      = VK_NULL_HANDLE;
    PIPELINE_CREATE_INFO.subpass            = 0;

    VK_ASSERT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                        &PIPELINE_CREATE_INFO, NULL,
                                        &graphic_pipline->pipeline));

    vkDestroyShaderModule(device, vertex_module, NULL);
    vkDestroyShaderModule(device, frag_module, NULL);
}

void recreate_swapchain(Region_Alloc* region, Application_State* app_state,
                        uint32 width, uint32 height)
{
    vkDeviceWaitIdle(app_state->device);

    for (uint32 i = 0; i < app_state->swap_chain.num_images; i++)
    {
        vkDestroyFramebuffer(app_state->device,
                             app_state->swap_chain.framebuffers[i], NULL);
        vkDestroyImageView(app_state->device,
                           app_state->swap_chain.img_views[i], NULL);
    }
    vkDestroySwapchainKHR(app_state->device, app_state->swap_chain.swap_chain,
                          NULL);

    vkDestroyRenderPass(app_state->device, app_state->swap_chain.render_pass,
                        NULL);

    for (uint32 i = 0; i < size_arr(app_state->swap_chain.graphic_piplines);
         i++)
    {
        vkDestroyPipelineLayout(
            app_state->device, app_state->swap_chain.graphic_piplines[i].layout,
            NULL);
        vkDestroyPipeline(app_state->device,
                          app_state->swap_chain.graphic_piplines[i].pipeline,
                          NULL);
        vkDestroyDescriptorSetLayout(
            app_state->device,
            app_state->swap_chain.graphic_piplines[i].set_layout, NULL);
    }

    destroy_image(app_state->device, app_state->depth_img);

    create_swapchain(region, app_state->phy_device, app_state->device,
                     app_state->surface, width, height, app_state->q_indices,
                     &app_state->swap_chain);

    create_depth_image(app_state->device, app_state->phy_device,
                       app_state->swap_chain.extent_2D, &app_state->depth_img);

    get_swapchain_images(region, app_state->device, &app_state->swap_chain);

    create_render_pass(app_state->device, app_state->swap_chain.color_format,
                       &app_state->swap_chain.render_pass);

    get_head(app_state->swap_chain.graphic_piplines)->size = 0;

    create_graphics_pipeline(
        region, app_state->device, app_state->swap_chain.color_format,
        app_state->swap_chain.render_pass, "Syntics/res/vert.spv",
        "Syntics/res/frag.spv", app_state->swap_chain.extent_2D.width,
        app_state->swap_chain.extent_2D.height,
        &app_state->swap_chain.graphic_piplines[0]);

    get_head(app_state->swap_chain.graphic_piplines)->size++;

    create_graphics_pipeline(
        region, app_state->device, app_state->swap_chain.color_format,
        app_state->swap_chain.render_pass, "Syntics/res/gui.spv",
        "Syntics/res/gui.spv", app_state->swap_chain.extent_2D.width,
        app_state->swap_chain.extent_2D.height,
        &app_state->swap_chain.graphic_piplines[1]);

    get_head(app_state->swap_chain.graphic_piplines)->size++;

    assert(capacity_arr(app_state->swap_chain.img_views) ==
           app_state->swap_chain.num_images);

    assert(capacity_arr(app_state->swap_chain.framebuffers) ==
           app_state->swap_chain.num_images);

    for (uint32 i = 0; i < app_state->swap_chain.num_images; i++)
    {
        create_image_view(
            app_state->device, app_state->swap_chain.images[i],
            VK_IMAGE_VIEW_TYPE_2D, app_state->swap_chain.color_format,
            VK_IMAGE_ASPECT_COLOR_BIT, &app_state->swap_chain.img_views[i]);

        create_frame_buffer(
            app_state->device, app_state->swap_chain.render_pass,
            app_state->swap_chain.extent_2D, app_state->swap_chain.img_views[i],
            app_state->depth_img.img_view,
            &app_state->swap_chain.framebuffers[i]);
    }
}

} // namespace synt
