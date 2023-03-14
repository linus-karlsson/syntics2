#include "vulkan_api.h"
#include "buffers.h"
#include "instance_device.h"
#include "swap_chain.h"
#include "render.h"
#include "region_alloc.h"
#include "win32/win32_platform.h"
#include "vulkan_types.h"
#include <stdlib.h>
#include <string.h>

static Application_State* internal_handle = NULL;
static b8 INITIALIZED = false;

void init_vulkan(Region_Alloc* region, Application_State* app_state, u32 width,
                 u32 height)
{
    if (INITIALIZED) SY_ERROR("Already initialized vulkan");

    init_instance(region);
    if (VALIDATIONS_ENABLE) init_debug_messenger();

    create_surface(get_win(), &app_state->surface);

    pick_physical_device(region, get_instance(), app_state->surface,
                         &app_state->phy_device, &app_state->q_indices);

    create_logical_device(app_state->phy_device, app_state->q_indices,
                          &app_state->device);

    Queues queue = { 0 };
    vkGetDeviceQueue(app_state->device, app_state->q_indices.indices[GRAPHICS_QUEUE_IDX],
                     0, &queue.graphic_queue);

    vkGetDeviceQueue(app_state->device, app_state->q_indices.indices[GRAPHICS_QUEUE_IDX],
                     0, &queue.present_queue);

    create_command_pool(app_state->device,
                        app_state->q_indices.indices[GRAPHICS_QUEUE_IDX],
                        &app_state->com_pool);

#if 0
    load_vertices_indices(region, app_state, queue.graphic_queue);
#endif

    create_swapchain(app_state->phy_device, app_state->device, app_state->surface, width,
                     height, app_state->q_indices, &app_state->swap_chain);

    enable_multisample(&app_state->swap_chain, app_state->device, app_state->phy_device,
                       &app_state->color_img);

    create_depth_image(app_state->device, app_state->phy_device,
                       &app_state->swap_chain.extent_2D,
                       app_state->swap_chain.sample_count, &app_state->depth_img);

    get_swapchain_images(region, app_state->device, &app_state->swap_chain);

    create_render_pass(app_state->device, app_state->swap_chain.color_format,
                       app_state->swap_chain.sample_count,
                       &app_state->swap_chain.render_pass);

    app_state->swap_chain.img_views =
        dyn_arrayP(region, app_state->swap_chain.num_images, VkImageView);

    app_state->swap_chain.framebuffers =
        dyn_arrayP(region, app_state->swap_chain.num_images, VkFramebuffer);

    for (u32 i = 0; i < app_state->swap_chain.num_images; i++)
    {
        create_image_view(app_state->device, app_state->swap_chain.images[i],
                          VK_IMAGE_VIEW_TYPE_2D, app_state->swap_chain.color_format,
                          VK_IMAGE_ASPECT_COLOR_BIT, 1,
                          &app_state->swap_chain.img_views[i]);

        create_frame_buffer(app_state->device, app_state->swap_chain.render_pass,
                            app_state->swap_chain.extent_2D,
                            app_state->swap_chain.img_views[i],
                            app_state->depth_img.img_view, app_state->color_img.img_view,
                            &app_state->swap_chain.framebuffers[i]);
    }

    app_state->num_semaphores = 2;
    init_render_state(region, app_state->device, queue, app_state->phy_device,
                      app_state->com_pool, &app_state->q_indices,
                      app_state->num_semaphores, &app_state->swap_chain);

    internal_handle = app_state;
    INITIALIZED = true;
}

void destroy_vulkan()
{
    vkDeviceWaitIdle(internal_handle->device);

    for (u32 i = 0; i < internal_handle->swap_chain.num_images; i++)
    {
        vkDestroyFramebuffer(internal_handle->device,
                             internal_handle->swap_chain.framebuffers[i], NULL);
        vkDestroyImageView(internal_handle->device,
                           internal_handle->swap_chain.img_views[i], NULL);
    }
    vkDestroySwapchainKHR(internal_handle->device, internal_handle->swap_chain.swap_chain,
                          NULL);

    vkDestroyRenderPass(internal_handle->device, internal_handle->swap_chain.render_pass,
                        NULL);

    destroy_render_state();

    vkDestroyCommandPool(internal_handle->device, internal_handle->com_pool, NULL);

    destroy_image(internal_handle->device, internal_handle->color_img);
    destroy_image(internal_handle->device, internal_handle->depth_img);

    vkDestroyDevice(internal_handle->device, NULL);

    vkDestroySurfaceKHR(get_instance(), internal_handle->surface, NULL);
    destroy_instance();
}

