#ifndef SY_UNIT_BUILD
#include "vulkan_api.h"
#include "logging.h"
#include "instance_device.h"
#include "application.h"
#include "buffers.h"
#include "swap_chain.h"
#include "render.h"
#endif

global b8 VULKAN_API_INITIALIZED;
void vulkan_init(Region_Alloc* region, Instance_State* instance_state,
                 Application_State* app_state, Render_State** render_state,
                 u32 width, u32 height)
{
    assert(!VULKAN_API_INITIALIZED);
    if (vulkan_enable_validation()) vulkan_debug_messenger_init(instance_state);

    vulkan_surface_create(app_state->platform, instance_state->instance,
                          &app_state->surface);

    vulkan_pick_physical_device(instance_state->instance, app_state->surface,
                                &app_state->phy_device, &app_state->q_indices);

    vulkan_logical_device_create(app_state->phy_device, app_state->q_indices,
                                 &app_state->device);

    Queues queue = { 0 };
    vkGetDeviceQueue(app_state->device,
                     app_state->q_indices.indices[GRAPHICS_QUEUE_IDX], 0,
                     &queue.graphic_queue);

    vkGetDeviceQueue(app_state->device,
                     app_state->q_indices.indices[GRAPHICS_QUEUE_IDX], 0,
                     &queue.present_queue);

    vulkan_command_pool_create(app_state->device,
                               app_state->q_indices.indices[GRAPHICS_QUEUE_IDX],
                               &app_state->com_pool);

#if 0
    load_vertices_indices(region, app_state, queue.graphic_queue);
#endif

    vulkan_swapchain_create(app_state->phy_device, app_state->device,
                            app_state->surface, width, height,
                            app_state->q_indices, VK_NULL_HANDLE, false,
                            &app_state->swap_chain);

    vulkan_enable_multisample(&app_state->swap_chain, app_state->device,
                              app_state->phy_device, &app_state->color_img);

    vulkan_depth_image_create(app_state->device, app_state->phy_device,
                              &app_state->swap_chain.extent_2D,
                              app_state->swap_chain.sample_count,
                              &app_state->depth_img);

    vulkan_swapchain_get_images(region, app_state->device,
                                &app_state->swap_chain);

    vulkan_render_pass_create(
        app_state->device, app_state->swap_chain.color_format,
        app_state->swap_chain.sample_count, &app_state->swap_chain.render_pass);

    app_state->swap_chain.img_views =
        region_array(region, app_state->swap_chain.num_images, VkImageView);

    app_state->swap_chain.framebuffers =
        region_array(region, app_state->swap_chain.num_images, VkFramebuffer);

    for (u32 i = 0; i < app_state->swap_chain.num_images; i++)
    {
        vulkan_image_view_create(
            app_state->device, app_state->swap_chain.images[i],
            VK_IMAGE_VIEW_TYPE_2D, app_state->swap_chain.color_format,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, &app_state->swap_chain.img_views[i]);

        vulkan_frame_buffer_create(
            app_state->device, app_state->swap_chain.render_pass,
            app_state->swap_chain.extent_2D, app_state->swap_chain.img_views[i],
            app_state->depth_img.img_view, app_state->color_img.img_view,
            &app_state->swap_chain.framebuffers[i]);
    }

    vulkan_render_state_init(
        region, app_state->device, queue, app_state->phy_device,
        app_state->com_pool, &app_state->q_indices, app_state->num_semaphores,
        &app_state->swap_chain, app_state->platform, render_state);

    VULKAN_API_INITIALIZED = true;
}

void vulkan_destroy(Application_State* app_state, Render_State* render_state)
{
    vkDeviceWaitIdle(app_state->device);

    for (u32 i = 0; i < app_state->swap_chain.num_images; i++)
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

    vulkan_render_state_destroy(app_state->device, render_state);

    vkDestroyCommandPool(app_state->device, app_state->com_pool, NULL);

    vulkan_image_destroy(app_state->device, app_state->color_img);
    vulkan_image_destroy(app_state->device, app_state->depth_img);

    // vkDestroyDevice(app_state->device, NULL);

    // vkDestroySurfaceKHR(instance_get(), app_state->surface, NULL);
    // instance_destroy();
}

