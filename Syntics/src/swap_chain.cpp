#include "swap_chain.h"
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
        VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                                            &surface_cap));

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
            if (surface_formats.data[i].format == VK_FORMAT_R8G8B8A8_SRGB &&
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
        if (surface_cap.minImageCount + 1 > surface_cap.maxImageCount &&
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

        VK_ASSERT(
            vkCreateSwapchainKHR(device, &swap_info, NULL, &swap_chain->swap_chain));
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
        VkAttachmentDescription attachment_descs[1] = {};

        VkAttachmentDescription color_attach_desc = {};
        color_attach_desc.format                  = color_format;
        color_attach_desc.samples                 = VK_SAMPLE_COUNT_1_BIT;
        color_attach_desc.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attach_desc.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
        color_attach_desc.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attach_desc.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attach_desc.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attach_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachment_descs[0] = color_attach_desc;

        VkAttachmentReference color_attach_ref = {};
        color_attach_ref.attachment            = 0;
        color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_desc = {};
        subpass_desc.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.colorAttachmentCount = 1;
        subpass_desc.pColorAttachments    = &color_attach_ref;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = sy_size(attachment_descs);
        render_pass_info.pAttachments    = attachment_descs;
        render_pass_info.subpassCount    = 1;
        render_pass_info.pSubpasses      = &subpass_desc;

        VK_ASSERT(vkCreateRenderPass(device, &render_pass_info, NULL, render_pass));
    }

    void get_swapchain_images(Region_Alloc* region, VkDevice device,
                              Swap_Chain_attrib* swap_chain)
    {

        vkGetSwapchainImagesKHR(device, swap_chain->swap_chain,
                                &swap_chain->num_images, NULL);

        if (!swap_chain->images)
            swap_chain->images =
                dyn_array(region, swap_chain->num_images, VkImage, synt::PERM_ARRAY);

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
        view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
        view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
        view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
        view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
        view_create_info.subresourceRange.aspectMask = aspect_mask;
        view_create_info.subresourceRange.levelCount = 1;
        view_create_info.subresourceRange.layerCount = 1;

        VK_ASSERT(vkCreateImageView(device, &view_create_info, NULL, image_view));
    }

    void create_frame_buffer(VkDevice device, VkRenderPass render_pass,
                             VkExtent2D extent_2D, VkImageView img_view,
                             VkFramebuffer* framebuffer)
    {
        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass      = render_pass;
        framebuffer_info.renderPass      = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments    = &img_view;
        framebuffer_info.width           = extent_2D.width;
        framebuffer_info.height          = extent_2D.height;
        framebuffer_info.layers          = 1;

        VK_ASSERT(vkCreateFramebuffer(device, &framebuffer_info, NULL, framebuffer));
    }

    void create_graphics_pipeline(Region_Alloc* region, VkDevice device,
                                  VkFormat format, const char* vert_path,
                                  const char* frag_path, uint32 width, uint32 height,
                                  Graphic_Pipline* graphic_pipline)
    {
        File_Attrib vert_file = read_file(region, vert_path, "rb");
        File_Attrib frag_file = read_file(region, frag_path, "rb");

        VkShaderModuleCreateInfo vertex_info = {};
        vertex_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertex_info.codeSize = vert_file.buffer.size();
        vertex_info.pCode    = (const uint32*)vert_file.buffer.data;

        VkShaderModuleCreateInfo frag_info = {};
        frag_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        frag_info.codeSize = frag_file.buffer.size();
        frag_info.pCode    = (const uint32*)frag_file.buffer.data;

        VkShaderModule vertex_module = VK_NULL_HANDLE;
        VkShaderModule frag_module   = VK_NULL_HANDLE;

        VK_ASSERT(vkCreateShaderModule(device, &vertex_info, NULL, &vertex_module));
        VK_ASSERT(vkCreateShaderModule(device, &frag_info, NULL, &frag_module));

        // Empty region stack
        frag_file.buffer.destroy();
        vert_file.buffer.destroy();

        VkPipelineShaderStageCreateInfo shader_stages[2] = {};

        shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stages[0].module = vertex_module;

        shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stages[1].module = frag_module;

        // TODO: Probably need min and max depth.

        VkViewport view_port = {};
        view_port.width      = width;
        view_port.height     = height;

        VkRect2D scissor      = {};
        scissor.extent.width  = width;
        scissor.extent.height = height;

        VkPipelineViewportStateCreateInfo view_port_info = {};
        view_port_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        view_port_info.viewportCount = 1;
        view_port_info.pViewports    = &view_port;
        view_port_info.scissorCount  = 1;
        view_port_info.pScissors     = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer_info = {};
        rasterizer_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer_info.cullMode    = VK_CULL_MODE_BACK_BIT;
        rasterizer_info.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer_info.lineWidth   = 1.0f;

        VkPipelineColorBlendStateCreateInfo color_blend_info = {};
        color_blend_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_info.logicOpEnable = VK_TRUE;
        color_blend_info.logicOp       = VK_LOGIC_OP_CLEAR;

        VkDescriptorSetLayoutBinding layout_binding = {};

        VkDescriptorSetLayoutCreateInfo set_layout_info = {};
        set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        // vkCreateDescriptorSetLayout(device, )

        VkPipelineLayoutCreateInfo layout_info = {};
        layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        // vkCreatePipelineLayout(device, )

        create_render_pass(device, format, &graphic_pipline->render_pass);

        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.renderPass = graphic_pipline->render_pass;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages    = shader_stages;
        pipeline_info.pViewportState      = &view_port_info;
        pipeline_info.pRasterizationState = &rasterizer_info;
        pipeline_info.pColorBlendState    = &color_blend_info;

        vkDestroyShaderModule(device, vertex_module, NULL);
        vkDestroyShaderModule(device, frag_module, NULL);
    }
} // namespace synt
